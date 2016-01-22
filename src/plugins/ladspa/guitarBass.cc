#include <ladspa.h>

// Nsound header
#include <Nsound/AudioStream.h>
#include <Nsound/Buffer.h>
#include <Nsound/GuitarBass.h>

static const unsigned int nsound_base_id = 2600;

//-----------------------------------------------------------------------------
enum PortNumbers
{
    FREQUENCY,
    OUTPUT,
    MAX_PORT
};

static const char * PortNames[MAX_PORT] =
{
    "Frequency (Hz)",
    "Output Signal"
};

static const LADSPA_PortDescriptor port_descriptors[MAX_PORT] =
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO
};

static const LADSPA_PortRangeHint port_hints[MAX_PORT] =
{
    // frequency
    { LADSPA_HINT_BOUNDED_BELOW |
      LADSPA_HINT_BOUNDED_ABOVE |
      LADSPA_HINT_DEFAULT_LOW, 0.0, 2000.0
    }

};

//-----------------------------------------------------------------------------
class PluginData
{
    public:

    float sample_rate_;
    float frequency_;
    LADSPA_Data * y_;

    Nsound::GuitarBass * nsound_guitar_bass_;
};

//-----------------------------------------------------------------------------
// This gets called after the ports are connected and before the plugin is
// actually executed.
void
activatePlugin(LADSPA_Handle instance)
{
    #ifdef DEBUG
        printf("%s:%d: activatePlugin()\n", __FILE__, __LINE__);
    #endif

    PluginData * plugin = (PluginData *) instance;

    Nsound::float64 sr   = static_cast<Nsound::float64>(plugin->sample_rate_);

    plugin->nsound_guitar_bass_ = new Nsound::GuitarBass(sr);

    if(plugin->nsound_guitar_bass_ == NULL)
    {
        printf("%s:%d: failed to allocate Nsound::GuitarBass\n",
            __FILE__, __LINE__);
    }
}

//-----------------------------------------------------------------------------
// This gets called after the plugin processes the data.
void
deactivatePlugin(LADSPA_Handle instance)
{
    #ifdef DEBUG
        printf("%s:%d: deactivatePlugin()\n", __FILE__, __LINE__);
    #endif

    PluginData * plugin = (PluginData *) instance;

    delete plugin->nsound_guitar_bass_;
}

//-----------------------------------------------------------------------------
void
cleanupPlugin(LADSPA_Handle instance)
{
    #ifdef DEBUG
        printf("%s:%d: cleanupPlugin()\n", __FILE__, __LINE__);
    #endif
    PluginData * plugin = (PluginData *) instance;

    delete plugin;
}

//-----------------------------------------------------------------------------
void
connectPlugin(
    LADSPA_Handle instance,
    unsigned long port,
    LADSPA_Data * data_location)
{
    #ifdef DEBUG
        printf("%s:%d: connectPlugin()\n", __FILE__, __LINE__);
    #endif

    PluginData * plugin;

    plugin = (PluginData *)instance;

    switch(port)
    {
//~        case DURATION:
//~            plugin->duration_ = *data_location;
//~            break;

        case FREQUENCY:
            plugin->frequency_ = *data_location;
            break;

        case OUTPUT:
            plugin->y_ = data_location;
            break;

        default:
            printf("%s:%d: connectPlugin(): Port not connected: %d\n",
                __FILE__, __LINE__, port);
    }
}

//-----------------------------------------------------------------------------
LADSPA_Handle
instantiatePlugin(
    const LADSPA_Descriptor * descriptor,
    unsigned long sample_rate)
{
    #ifdef DEBUG
        printf("%s:%d: instantiatePlugin()\n", __FILE__, __LINE__);
        printf("%s:%d: instantiatePlugin()::sample_rate = %d\n",
            __FILE__, __LINE__, sample_rate);
    #endif

    PluginData * plugin = new PluginData();

    plugin->sample_rate_ = (float) sample_rate;

    return (LADSPA_Handle) plugin;
}

//-----------------------------------------------------------------------------
void
runPlugin(
    LADSPA_Handle instance,
    unsigned long sample_count)
{
    #ifdef DEBUG
        printf("%s:%d: runPlugin(sample_count = %d)\n",
            __FILE__, __LINE__, sample_count);
    #endif

    PluginData * plugin = (PluginData *) instance;

    Nsound::AudioStream y(plugin->sample_rate_, 1);

    float duration = static_cast<float>(sample_count) / plugin->sample_rate_;

    y = plugin->nsound_guitar_bass_->play(
        duration,
        plugin->frequency_);

    unsigned long n_samples = sample_count;

    if(n_samples > y.getLength())
    {
        n_samples = y.getLength();
    }

    for(unsigned long n = 0; n < y.getLength(); ++n)
    {
        plugin->y_[n] = static_cast<LADSPA_Data>(y[0][n]);
    }
}

//-----------------------------------------------------------------------------
LADSPA_Descriptor * descriptor = NULL;

void
new_descriptor()
{
    descriptor = new LADSPA_Descriptor();

    descriptor->UniqueID = nsound_base_id + 2;
    descriptor->Label = "Nsound";
    descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    descriptor->Name = "Nsound Guitar Bass";
    descriptor->Maker = "Nick Hilton <weegreenblobbie_yahoo_com>";
    descriptor->Copyright = "GPL";
    descriptor->PortCount = MAX_PORT;

    descriptor->PortDescriptors = port_descriptors;
    descriptor->PortRangeHints = port_hints;
    descriptor->PortNames = PortNames;

    descriptor->activate = activatePlugin;
    descriptor->cleanup = cleanupPlugin;
    descriptor->connect_port = connectPlugin;
    descriptor->deactivate = deactivatePlugin;
    descriptor->instantiate = instantiatePlugin;
    descriptor->run = runPlugin;
    descriptor->run_adding = NULL;
    descriptor->set_run_adding_gain = NULL;
}

//-----------------------------------------------------------------------------
const LADSPA_Descriptor *
ladspa_descriptor (unsigned long index)
{
    #ifdef DEBUG
        printf("%s:%d: ladspa_descriptor(%d)\n", __FILE__, __LINE__, index);
    #endif

    if(index == 0)
    {
        if(descriptor == NULL)
        {
            new_descriptor();
        }

        return descriptor;
    }

    return NULL;
}
