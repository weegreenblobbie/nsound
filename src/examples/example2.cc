#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <Nsound/NsoundAll.h>

#include <map>
#include <string>
#include <sstream>

void RestoreKeyboardBlocking(struct termios *initial_settings)
{
	tcsetattr(0, TCSANOW, initial_settings);
}

void SetKeyboardNonBlock(struct termios *initial_settings)
{

    struct termios new_settings;
    tcgetattr(0,initial_settings);

    new_settings = *initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &new_settings);
}

char read_char()
{
    char buff[2];
    int l = read(STDIN_FILENO,buff,1);
    if (l>0) return buff[0];
    return (EOF);
}

std::map<std::string, float> notes = {
     {"A0", 27.50000},
     {"B0", 30.86771},
     {"C1", 32.70320},
     {"D1", 36.70810},
     {"E1", 41.20344},
     {"F1", 43.65353},
     {"G1", 48.99943}, // ???
     {"A1", 55.00000}, // ???
     {"B1", 61.73541}, // ???
     {"C2", 65.40639},
     {"D2", 73.41619},
     {"E2", 82.40689},
     {"F2", 87.30706},
     {"G2", 97.99886},
     {"A2",110.0000 },
     {"B2",123.4708 },
     {"C3",130.8128 },
     {"D3",146.8324 },
     {"E3",164.8138 },
     {"F3",174.6141 },
     {"G3",195.9977 },
     {"A3",220.0000 }, // ??
     {"B3",246.9417 }, // ??
     {"C4",261.6256 },
     {"D4",293.6648 },
     {"E4",329.6276 }, // 1.0
     {"F4",349.2282 },
     {"G4",391.9954 },
     {"A4",440.0000 },
     {"B4",493.8833 },
     {"C5",523.2511 },
     {"D5",587.3295 },
     {"E5",659.2551 },
     {"F5",698.4565 },
     {"G5",783.9909 },
     {"A5",880.0000 },
     {"B5",987.7666 },
     {"C6",1046.502 },
     {"D6",1174.659 },
     {"E6",1318.510 },
     {"F6",1396.913 },
     {"G6",1567.982 },
     {"A6",1760.000 },
     {"B6",1975.533 }, // ??
     {"C7",2093.005 },
     {"D7",2349.318 },
     {"E7",2637.020 },
     {"F7",2793.826 },
     {"G7",3135.963 },
     {"A7",3520.000 },
     {"B7",3951.066 }, // ??
     {"C8",4186.009 }, // ??
};

using namespace Nsound;


int main(int argc, char *argv[])
{
    std::string arg1 = "C1";
    float scale = 1.0;

    if (argc >= 2)
    {
        arg1 = std::string(argv[1]);
        if (notes.count(arg1) == 0)
        {
            std::cerr << "Could not find note " << arg1 << "\n";
            return 1;
        }
    }

    if (argc >= 3)
    {
        std::stringstream ss(argv[2]);
        ss >> scale;
    }

    std::cout << "Note: " << arg1 << ": " << scale << "\n";

    struct termios term_settings;
    SetKeyboardNonBlock(&term_settings);

    const float64 sr = 44100.0;

    OrganPipe po(sr);

    AudioStream ref(sr, 2);
    AudioStream out(sr, 2);
    AudioStream silence(sr, 2);

    AudioPlayback playback(sr, 2);

    ref << po.play(1.0, notes["E4"]);
    out << po.play(1.0, notes[arg1]);
    silence = 0.0 * po.play(0.25, 1.0);

    float step = 0.05;

    char c = ' ';

    while (true)
    {
        if (c > 0)
        {
            if (c == 'q' || c == 'Q') break;

            if (c == 65)  // UP
            {
                scale += step;
                printf("scale = %.3f\n", scale);
            }

            if (c == 66)  // DOWN
            {
                scale -= step;
                printf("scale = %.3f\n", scale);
            }

            if (c == 67)  // Lower Step
            {
                step += 0.01;
                printf("step = %.3f\n", step);
            }
            if (c == 68)  // Higher Step
            {
                step -= 0.01;
                if (step < 0.0) step = 0.01;
                printf("step = %.3f\n", step);
            }

            fflush(stdout);

            if (c == ' ')
            {
                ref >> playback;
                silence >> playback;
                scale * out >> playback;
                silence >> playback;
            }
        }

        c = read_char();
    }

    RestoreKeyboardBlocking(&term_settings);

    std::cout << "Note " << arg1 << ": " << scale << "\n";

    return 0;
}
