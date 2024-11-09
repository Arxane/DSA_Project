#include <FL/Fl_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Chart.H>
#include <vector>
#include <iostream>
#include <chrono>
#include <cstdio>
#include <sstream>
#include "Encode.h"
#include "Decode.h"
#include <iomanip>
using namespace std;

class CompressionGraph : public Fl_Widget {
private:
    vector<pair<long long, long long>> points; // Input size, Output size pairs
    long long max_size;

public:
    CompressionGraph(int x, int y, int w, int h, const char* l = 0) 
        : Fl_Widget(x, y, w, h, l), max_size(1) {}

    void clear_points() {
        points.clear();
        max_size = 1;
        redraw();
    }

 void draw() override {
    // Draw background
    fl_color(FL_WHITE);
    fl_rectf(x(), y(), w(), h());
    
    // Draw border
    fl_color(FL_BLACK);
    fl_rect(x(), y(), w(), h());

    // Draw axes
    int margin = 50;  // Increased margin for scale labels
    int graph_x = x() + margin;
    int graph_y = y() + h() - margin;
    int graph_w = w() - 2 * margin;
    int graph_h = h() - 2 * margin;

    fl_line_style(FL_SOLID, 2);
    fl_line(graph_x, graph_y, graph_x + graph_w, graph_y);  // X axis
    fl_line(graph_x, graph_y, graph_x, graph_y - graph_h);  // Y axis

    // Draw labels
    fl_font(FL_HELVETICA, 12);
    fl_color(FL_BLACK);
    fl_draw("Input Size (KB)", graph_x + graph_w/3, graph_y + 25);
    
    // Drawing Y-axis label vertically
    const char* y_label = "Output Size (KB)";
    int char_height = fl_height();
    int y_start = graph_y - graph_h/2 - (strlen(y_label) * char_height)/2;
    for(const char* c = y_label; *c; c++) {
        char label[2] = {*c, '\0'};
        fl_draw(label, graph_x - 45, y_start);
        y_start += char_height;
    }

    // Drawing grid lines and scale
    fl_color(fl_rgb_color(200, 200, 200));
    char scale_label[32];
    fl_font(FL_HELVETICA, 10);
    
    // Drawing vertical grid lines and X-axis scale
    for(int i = 0; i <= 4; i++) {
        fl_line_style(FL_DASH, 1);
        int x_pos = graph_x + (i * graph_w/4);
        fl_line(x_pos, graph_y, x_pos, graph_y - graph_h);
        
        // Drawing X-axis scale (in KB)
        double x_value = (max_size * i) / (4.0 * 1024); // Convert to KB
        snprintf(scale_label, sizeof(scale_label), "%.1f", x_value);
        fl_color(FL_BLACK);
        fl_draw(scale_label, x_pos - 15, graph_y + 15);
    }

    // Drawing horizontal grid lines and Y-axis scale
    for(int i = 0; i <= 4; i++) {
        fl_color(fl_rgb_color(200, 200, 200));
        fl_line_style(FL_DASH, 1);
        int y_pos = graph_y - (i * graph_h/4);
        fl_line(graph_x, y_pos, graph_x + graph_w, y_pos);
        
        // Draw Y-axis scale (in KB)
        double y_value = (max_size * i) / (4.0 * 1024); // Convert to KB
        snprintf(scale_label, sizeof(scale_label), "%.1f", y_value);
        fl_color(FL_BLACK);
        fl_draw(scale_label, graph_x - 35, y_pos + 5);
    }

    // Drawing points and line
    if (!points.empty()) {
        fl_color(FL_RED);
        fl_line_style(FL_SOLID, 2);
        fl_begin_line();
        for (const auto& point : points) {
            int px = graph_x + (point.first * graph_w) / max_size;
            int py = graph_y - (point.second * graph_h) / max_size;
            fl_circle(px, py, 4);
            fl_vertex(px, py);
        }
        fl_end_line();

        // Drawing compression ratio with KB units
        if (points.size() >= 2) {
            auto& last_point = points.back();
            double ratio = (double)last_point.second / last_point.first * 100;
            double input_kb = last_point.first / 1024.0;
            double output_kb = last_point.second / 1024.0;
            char ratio_str[100];
            snprintf(ratio_str, sizeof(ratio_str), 
                    "Last Compression: %.2f KB → %.2f KB (%.1f%%)", 
                    input_kb, output_kb, ratio);
            fl_color(FL_BLACK);
            fl_draw(ratio_str, graph_x + 10, y() + 20);
        }
    }
}

// Updating the add_point method to store sizes in bytes but display in KB
void add_point(long long input_size, long long output_size) {
    points.push_back({input_size, output_size});
    max_size = 1024; // Start with minimum 1KB scale
    
    // Finding the maximum size in bytes to set the scale
    for (const auto& point : points) {
        max_size = max(max_size, max(point.first, point.second));
    }
    
    // Round up max_size to the next nice number
    long long scale_kb = (max_size + 1023) / 1024;  // Converting to KB and round up
    long long nice_scale = 1;
    while (nice_scale < scale_kb) {
        nice_scale *= 2;
    }
    max_size = nice_scale * 1024;  // Converting back to bytes
    
    redraw();
}
};

class FileCompressorDecompressor {
public:
    Fl_Window *window;
    Fl_Button *compress_button;
    Fl_Button *decompress_button;
    Fl_Button *clear_graph_button;
    Fl_Input *input_file_path;
    Fl_Input *output_file_path;
    Fl_Box *header;
    Fl_Text_Display *status_display;
    Fl_Text_Buffer *status_buffer;
    CompressionGraph *graph;

    FileCompressorDecompressor(int width, int height, const char *title)
        : window(new Fl_Window(width, height, title)),
          compress_button(new Fl_Button(50, 150, 150, 40, "Compress File")),
          decompress_button(new Fl_Button(210, 150, 150, 40, "Decompress File")),
          clear_graph_button(new Fl_Button(370, 150, 150, 40, "Clear Graph")),
          input_file_path(new Fl_Input(150, 50, 300, 30, "Input File Path:")),
          output_file_path(new Fl_Input(150, 100, 300, 30, "Output File Path:")),
          header(new Fl_Box(FL_NO_BOX, 50, 10, 400, 40, "Huffman Compressor/Decompressor")),
          status_buffer(new Fl_Text_Buffer()),
          status_display(new Fl_Text_Display(50, 460, 500, 120, "Status:")),
          graph(new CompressionGraph(50, 210, 500, 230, "Compression Ratio Graph")) {

        // Set window settings
        window->size(width, height);
        window->color(fl_rgb_color(245, 245, 250));
        window->resizable(window);

        // Header settings
        header->align(FL_ALIGN_CENTER);
        header->labelfont(FL_BOLD);
        header->labelsize(24);
        header->labelcolor(FL_BLACK);

        // Input/output file path settings
        input_file_path->labelfont(FL_BOLD);
        output_file_path->labelfont(FL_BOLD);

        // Status display settings
        status_display->buffer(status_buffer);
        status_display->textfont(FL_COURIER);
        status_display->textsize(20);
        status_display->color(FL_BLACK);
        status_display->textcolor(FL_GREEN);

        // Style buttons
        customize_button(compress_button, FL_ROUNDED_BOX, 16, fl_rgb_color(189, 6, 10), FL_WHITE);
        customize_button(decompress_button, FL_ROUNDED_BOX, 16, fl_rgb_color(189, 6, 10), FL_WHITE);
        customize_button(clear_graph_button, FL_ROUNDED_BOX, 16, fl_rgb_color(100, 100, 100), FL_WHITE);

        // Callbacks
        compress_button->callback(Compress_Callback, this);
        decompress_button->callback(Decompress_Callback, this);
        clear_graph_button->callback(Clear_Graph_Callback, this);

        window->end();
    }

    void show() {
        window->show();
    }

private:
    static long long getFileSize(const char* filename) {
        FILE* fp = fopen(filename, "rb");
        if (!fp) return -1;
        fseek(fp, 0, SEEK_END);
        long long size = ftell(fp);
        fclose(fp);
        return size;
    }

    static void updateStatus(FileCompressorDecompressor *fc, const string &message) {
        fc->status_buffer->append(message.c_str());
        fc->status_display->scroll(fc->status_buffer->length(), 0);
    }

    static void Clear_Graph_Callback(Fl_Widget *widget, void *data) {
        FileCompressorDecompressor *fc = (FileCompressorDecompressor *)data;
        fc->graph->clear_points();
        fc->status_buffer->text("Graph cleared\n");
    }

    static void Compress_Callback(Fl_Widget *widget, void *data) {
        FileCompressorDecompressor *fc = (FileCompressorDecompressor *)data;
        
        const char *input_file = fl_file_chooser("Select a text file", "*.txt", nullptr);
        if (!input_file) return;
        fc->input_file_path->value(input_file);
        long long initial_size = getFileSize(input_file);

        const char *output_file = fl_file_chooser("Save compressed file as", "*.huf", nullptr);
        if (!output_file) return;
        fc->output_file_path->value(output_file);

        fc->status_buffer->text("");
        
        if (initial_size == -1) {
            updateStatus(fc, "Error: Could not read input file size\n");
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();
        bool success = compressFile(fc->input_file_path->value(), fc->output_file_path->value());
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (success) {
            long long final_size = getFileSize(output_file);
            if (final_size == -1) {
                updateStatus(fc, "File compressed but couldn't read compressed size\n");
                return;
            }

            // Add point to graph
            fc->graph->add_point(initial_size, final_size);

            double ratio = (double)final_size / initial_size * 100;
            stringstream ss;
            ss << "Compression Successful!\n"
               << "Initial size: " << initial_size << " bytes\n"
               << "Final size: " << final_size << " bytes\n"
               << "Compression ratio: " << fixed << setprecision(2) << ratio << "%\n"
               << "Time taken: " << duration.count() << " ms\n\n";

            updateStatus(fc, ss.str());
        } else {
            updateStatus(fc, "Error: File compression failed\n\n");
        }
    }

    static void Decompress_Callback(Fl_Widget *widget, void *data) {
        FileCompressorDecompressor *fc = (FileCompressorDecompressor *)data;
        
        const char *input_file = fl_file_chooser("Select a compressed file", "*.huf", nullptr);
        if (!input_file) return;
        fc->input_file_path->value(input_file);
        long long initial_size = getFileSize(input_file);

        const char *output_file = fl_file_chooser("Save decompressed file as", "*.txt", nullptr);
        if (!output_file) return;
        fc->output_file_path->value(output_file);

        fc->status_buffer->text("");
        
        if (initial_size == -1) {
            updateStatus(fc, "Error: Could not read compressed file size\n");
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();
        bool success = decompressFile(fc->input_file_path->value(), fc->output_file_path->value());
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (success) {
            long long final_size = getFileSize(output_file);
            if (final_size == -1) {
                updateStatus(fc, "File decompressed but couldn't read decompressed size\n");
                return;
            }

            stringstream ss;
            ss << "Decompression Successful!\n"
               << "Compressed size: " << initial_size << " bytes\n"
               << "Decompressed size: " << final_size << " bytes\n"
               << "Expansion ratio: " << fixed << setprecision(2) 
               << ((double)final_size / initial_size * 100) << "%\n"
               << "Time taken: " << duration.count() << " ms\n\n";

            updateStatus(fc, ss.str());
        } else {
            updateStatus(fc, "Error: File decompression failed\n\n");
        }
    }

    static void customize_button(Fl_Button *button, Fl_Boxtype boxtype, int labelsize, 
                               Fl_Color color, Fl_Color labelcolor) {
        button->box(boxtype);
        button->labelsize(labelsize);
        button->color(color);
        button->labelcolor(labelcolor);
        button->when(FL_WHEN_CHANGED);
    }
};

int main() {
    FileCompressorDecompressor app(600, 600, "Huffman Compressor and Decompressor");
    app.show();
    return Fl::run();
}