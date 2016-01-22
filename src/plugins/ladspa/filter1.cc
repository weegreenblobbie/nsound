#include <ladspa.h>

// Nsound header
#include <Nsound/FilterLowPassIIR.h>

static const unsigned int nsound_base_id = 2600;

//-----------------------------------------------------------------------------
enum PortNumbers
{
    FILTER_ORDER = 0,
    FILTER_FREQUENCY,
    FILTER_RIPPLE,
    FILTER_INPUT,
    FILTER_OUTPUT,
    MAX_PORT
};

static const char * PortNames[MAX_PORT] =
{
    "Filter Order",
    "Cutt Off Frequency Hz",
    "Pass Band Ripple %",
    "Input Signal",
    "Output Signal"
};

static const LADSPA_PortDescriptor port_descriptors[MAX_PORT] =
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_CONTROL,
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO
};

static const LADSPA_PortRangeHint port_hints[MAX_PORT] =
{
    // order
    { LADSPA_HINT_BOUNDED_BELOW |
      LADSPA_HINT_BOUNDED_ABOVE |
      LADSPA_HINT_DEFAULT_1     |
      LADSPA_HINT_INTEGER, 0.9, 20.1
    },

    // frequency
    { LADSPA_HINT_BOUNDED_BELOW |
      LADSPA_HINT_BOUNDED_ABOVE |
      LADSPA_HINT_DEFAULT_440, 0.0, 24000.0
    },

    // ripple
    { LADSPA_HINT_BOUNDED_BELOW |
      LADSPA_HINT_BOUNDED_ABOVE |
      LADSPA_HINT_DEFAULT_0, 0.0, 1.0
    }
};




//-----------------------------------------------------------------------------
class PluginData
{
    public:

    float sample_rate_;
    float order_;
    float frequency_;
    float ripple_;
    LADSPA_Data * x_;
    LADSPA_Data * y_;

    Nsound::FilterLowPassIIR * nsound_filter_;
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

    Nsound::float64 sr    = static_cast<Nsound::float64>(plugin->sample_rate_);
    Nsound::uint32  order = static_cast<Nsound::uint32 >(plugin->order_);
    Nsound::float64 freq  = static_cast<Nsound::float64>(plugin->frequency_);
    Nsound::float64 rip   = static_cast<Nsound::float64>(plugin->ripple_);

    plugin->nsound_filter_ = new Nsound::FilterLowPassIIR(
        sr,
        order,
        freq,
        rip);

    if(plugin->nsound_filter_ == NULL)
    {
        printf("%s:%d: failed to allocate Nsound::FilterLowPassIIR\n",
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

    delete plugin->nsound_filter_;
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
        case FILTER_ORDER:
            plugin->order_ = *data_location;
            break;

        case FILTER_FREQUENCY:
            plugin->frequency_ = *data_location;
            break;

        case FILTER_RIPPLE:
            plugin->ripple_ = *data_location;
            break;

        case FILTER_INPUT:
            plugin->x_ = data_location;
            break;

        case FILTER_OUTPUT:
            plugin->y_ = data_location;
            break;
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

    for(unsigned long n = 0; n < sample_count; ++n)
    {
        Nsound::float64 y =
            plugin->nsound_filter_->filter(
                static_cast<Nsound::float64>(plugin->x_[n]));

        plugin->y_[n] = static_cast<LADSPA_Data>(y);
    }
}

//-----------------------------------------------------------------------------
LADSPA_Descriptor * descriptor = NULL;

void
new_descriptor()
{
    descriptor = new LADSPA_Descriptor();

    descriptor->UniqueID = nsound_base_id + 1;
    descriptor->Label = "Nsound";
    descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    descriptor->Name = "Nsound IIR Low Pass Filter";
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
