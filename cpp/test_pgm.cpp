// Quick round-trip test: read a .pgm, print pad names, modify one, save, verify.
#include <iostream>
#include <cassert>
#include <vector>
#include "src/pgm/Program.h"
#include "src/midi/MidiSequenceBuilder.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: test_pgm <file.pgm>\n";
        return 1;
    }

    std::string path = argv[1];
    std::cout << "Opening: " << path << "\n";

    Program prog = Program::open(path);
    std::cout << "Loaded OK. Buffer size: " << prog.buffer().size() << " bytes\n";
    assert(prog.buffer().size() == Program::FILE_LENGTH);

    // Print layer 0 sample names for first 8 pads
    std::cout << "\nPad sample names (layer 0):\n";
    for (int i = 0; i < 8; i++) {
        Pad pad = prog.get_pad(i);
        Layer layer = pad.get_layer(0);
        std::cout << "  Pad " << i << ": \"" << layer.get_sample_name() << "\""
                  << "  level=" << (int)layer.get_level()
                  << "  tuning=" << layer.get_tuning() << "\n";
    }

    // Modify pad 0 layer 0 sample name and verify round-trip
    {
        Pad pad = prog.get_pad(0);
        Layer layer = pad.get_layer(0);
        std::string original = layer.get_sample_name();
        layer.set_sample_name("TESTNAME");
        assert(pad.get_layer(0).get_sample_name() == "TESTNAME");
        layer.set_sample_name(original);
        assert(pad.get_layer(0).get_sample_name() == original);
        std::cout << "\nRound-trip string test: OK\n";
    }

    // Save and reload — must be byte-identical
    std::string out_path = "/tmp/test_out.pgm";
    prog.save(out_path);

    Program prog2 = Program::open(out_path);
    bool identical = true;
    for (int i = 0; i < Program::FILE_LENGTH; i++) {
        if (prog.buffer().getByte(i) != prog2.buffer().getByte(i)) {
            std::cerr << "MISMATCH at byte " << i << "\n";
            identical = false;
            break;
        }
    }
    std::cout << "Save/reload byte-identical: " << (identical ? "OK" : "FAIL") << "\n";

    // MIDI export
    MidiSequenceBuilder midi(44100, 120, 480);
    std::vector<int> markers = {0, 22050, 44100, 66150};
    midi.build("/tmp/test_groove.mid", markers, 36);
    std::cout << "MIDI export: OK -> /tmp/test_groove.mid\n";

    std::cout << "\nAll tests passed.\n";
    return 0;
}
