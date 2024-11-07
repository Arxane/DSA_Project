#include <FL/Fl_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <iostream>
#include "Encode.h"
#include "Decode.h"

using namespace std;

class FileCompressorDecompressor {
public:
    Fl_Window *window;
    Fl_Button *compress_button;
    Fl_Button *decompress_button;
    Fl_Input *input_file_path;
    Fl_Input *output_file_path;
    Fl_Box *header;

    FileCompressorDecompressor(int width, int height, const char *title)
        : window(new Fl_Window(width, height, title)),
          compress_button(new Fl_Button(150, 150, 200, 40, "Compress File")),
          decompress_button(new Fl_Button(150, 200, 200, 40, "Decompress File")),
          input_file_path(new Fl_Input(150, 50, 300, 30, "Input File Path:")),
          output_file_path(new Fl_Input(150, 100, 300, 30, "Output File Path:")),
          header(new Fl_Box(FL_NO_BOX, 50, 10, 400, 40, "Huffman Compressor/Decompressor")) {

        // Set window size and background color
        window->size(width, height);
        window->color(fl_rgb_color(245, 245, 250));  // #f5f5fa
        window->resizable(window);

        // Header settings
        header->align(FL_ALIGN_CENTER);
        header->labelfont(FL_BOLD);
        header->labelsize(24);
        header->labelcolor(FL_BLACK);

        // Input and output file path labels bold and black
        input_file_path->labelfont(FL_BOLD);
        input_file_path->labelsize(14);
        input_file_path->align(FL_ALIGN_LEFT);
        input_file_path->color(FL_WHITE);

        output_file_path->labelfont(FL_BOLD);
        output_file_path->labelsize(14);
        output_file_path->align(FL_ALIGN_LEFT);
        output_file_path->color(FL_WHITE);

        // Style buttons with rounded corners and hover effect
        customize_button(compress_button, FL_ROUNDED_BOX, 16, fl_rgb_color(189, 6, 10), FL_WHITE);  // #bd060a
        customize_button(decompress_button, FL_ROUNDED_BOX, 16, fl_rgb_color(189, 6, 10), FL_WHITE);

        // Callbacks
        compress_button->callback(Compress_Callback, this);
        decompress_button->callback(Decompress_Callback, this);

        window->end();
    }

    void show() {
        window->show();
    }

private:
    static void Compress_Callback(Fl_Widget *widget, void *data) {
        FileCompressorDecompressor *fc = (FileCompressorDecompressor *)data;
        const char *input_file = fl_file_chooser("Select a text file", "*.txt", nullptr);
        if (input_file) {
            fc->input_file_path->value(input_file);
        }
        const char *output_file = fl_file_chooser("Save compressed file as", "*.huf", nullptr);
        if (output_file) {
            fc->output_file_path->value(output_file);
        }
        if (compressFile(fc->input_file_path->value(), fc->output_file_path->value())) {
            cout << "File compressed successfully!\n";
        } else {
            cerr << "Error compressing the file.\n";
        }
    }

    static void Decompress_Callback(Fl_Widget *widget, void *data) {
        FileCompressorDecompressor *fc = (FileCompressorDecompressor *)data;
        const char *input_file = fl_file_chooser("Select a compressed file", "*.huf", nullptr);
        if (input_file) {
            fc->input_file_path->value(input_file);
        }
        const char *output_file = fl_file_chooser("Save decompressed file as", "*.txt", nullptr);
        if (output_file) {
            fc->output_file_path->value(output_file);
        }
        if (decompressFile(fc->input_file_path->value(), fc->output_file_path->value())) {
            cout << "File decompressed successfully!\n";
        } else {
            cerr << "Error decompressing the file.\n";
        }
    }

    // Helper function to customize button appearance
    static void customize_button(Fl_Button *button, Fl_Boxtype boxtype, int labelsize, Fl_Color color, Fl_Color labelcolor) {
        button->box(boxtype);
        button->labelsize(labelsize);
        button->color(color);
        button->labelcolor(labelcolor);
        button->when(FL_ENTER | FL_LEAVE);  // Set up hover effect
        button->callback(button_hover_effect);
    }

    // Hover effect for buttons
    static void button_hover_effect(Fl_Widget *widget, void *data) {
        Fl_Button *button = (Fl_Button *)widget;
        if (Fl::event() == FL_ENTER) {
            button->color(fl_rgb_color(150, 0, 0));  // Dark red on hover
            button->redraw();
        } else if (Fl::event() == FL_LEAVE) {
            button->color(fl_rgb_color(189, 6, 10));  // Original color
            button->redraw();
        }
    }
};

int main() {
    FileCompressorDecompressor app(600, 400, "Huffman Compressor/Decompressor");
    app.show();
    return Fl::run();
}
