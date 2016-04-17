#include <Nsound/NsoundAll.h>
#include <Nsound/biquad/FilterBank.hpp>

#include <iostream>

using std::cout;

using namespace Nsound;
using namespace Nsound::biquad;

int
main(void)
{

    AudioStream a("/home/nhilton/development/nsound/voice_samples/susan_blackmore-what-is-it-like-to-be-a-bat.wav");

    Buffer b = a[0];

    uint32 n = b.getLength();

    CircularBuffer cb(n);

    cb.write(b);

    n = 441;

    AudioStream aout(a.getSampleRate(), 1);

    for(uint32 i = 0; i < b.getLength() / n + 50; ++i)
    {
        auto temp = Buffer::zeros(n);

        cb.read_rt(temp);

        aout << temp;
    }

    aout >> "output.wav";

    return 0;
}
