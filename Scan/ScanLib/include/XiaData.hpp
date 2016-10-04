#ifndef XIADATA_HPP
#define XIADATA_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>

/*! \brief A pixie16 channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * identifier, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable.
 */
class XiaData{
public:
    double energy; /// Raw pixie energy.
    double time; /// Raw pixie event time. Measured in filter clock ticks (8E-9 Hz for RevF).
    
    size_t traceLength;
    unsigned short *adcTrace; /// ADC trace capture.
    
    size_t numQdcs; /// Number of QDCs onboard.
    unsigned int *qdcValue; /// QDCs from onboard.
    
    unsigned int slotNum; ///Slot number
    unsigned int modNum; /// Module number.
    unsigned int chanNum; /// Channel number.
    unsigned int trigTime; /// The channel trigger time, trigger time and the lower 32 bits of the event time are not necessarily the same but could be separated by a constant value.
    unsigned int cfdTime; /// CFD trigger time in units of 1/256 pixie clock ticks.
    unsigned int eventTimeLo; /// Lower 32 bits of pixie16 event time.
    unsigned int eventTimeHi; /// Upper 32 bits of pixie16 event time.
    double eventTime; /// The event time recorded by Pixie.
    
    bool virtualChannel; /// Flagged if generated virtually in Pixie DSP.
    bool pileupBit; /// Pile-up flag from Pixie.
    bool saturatedBit; /// Saturation flag from Pixie.
    bool cfdForceTrig; /// CFD was forced to trigger.
    bool cfdTrigSource; /// The ADC that the CFD/FPGA synched with.
    
    /// Default constructor.
    XiaData();
    
    /// Constructor from a pointer to another XiaData.
    XiaData(XiaData *other_);
    
    /// Virtual destructor.
    virtual ~XiaData();
    
    /// Get the event ID number (mod * chan).
    int getID(){ return(modNum*16+chanNum); }
    
    /// Reserve specified number of bins for the channel trace.
    void reserve(const size_t &size_);
    
    /// Fill the trace vector with a specified value.
    void assign(const unsigned short &input_);

	/// Fill the trace by reading from a character array.
	void copyTrace(char *ptr_);

	/// Fill the trace by reading from a character array.
	void copyTrace(char *ptr_, const size_t &size_);	

	/// Fill the QDC array by reading a character array.
	void copyQDCs(char *ptr_, const size_t &size_);

    /// Return true if the time of arrival for rhs is later than that of lhs.
    static bool compareTime(XiaData *lhs, XiaData *rhs){ return (lhs->time < rhs->time); }
    
    /// Return true if lhs has a lower event id (mod * chan) than rhs.
    static bool compareChannel(XiaData *lhs, XiaData *rhs){ return ((lhs->modNum*lhs->chanNum) < (rhs->modNum*rhs->chanNum)); }
    
    /// Return one of the onboard qdc values.
    unsigned int getQdcValue(const size_t &id){ return (id < 0 || id >= numQdcs ? -1 : qdcValue[id]); }
    
    /// Clear all variables.
    void clear();
    
    /// Delete the trace.
    void clearTrace();
    
    /// Delete the QDC array.
    void clearQDCs();
    
    /// Write a pixie style event to a binary output file.
    int writeRaw(std::ofstream &file_);
};

class ChannelEvent : public XiaData {
public:
	bool write_trace; /// True if the adc trace is to be written to binary output file (presort only).
    bool valid_chan; /// True if the high resolution energy and time are valid.
    bool ignore; /// Ignore this event.
    
    float phase; /// Phase (leading edge) of trace (in ADC clock ticks (4E-9 Hz for 250 MHz digitizer)).
    float baseline; /// The baseline of the trace.
    float stddev; /// Standard deviation of the baseline.
    float maximum; /// The baseline corrected maximum value of the trace.
    float qdc; /// The calculated (baseline corrected) qdc.
    size_t max_index; /// The index of the maximum trace bin (in ADC clock ticks).
    
    float *cfdvals; ///
    
    /// Default constructor.
    ChannelEvent();
    
    /// Constructor from a XiaData. ChannelEvent will take ownership of the XiaData.
    ChannelEvent(XiaData *event_);
    
    /// Destructor.
    ~ChannelEvent();
    
    /// Calculate the trace baseline, baseline standard deviation, and find the pulse maximum.
    float ComputeBaseline();
    
    /// Integrate the baseline corrected trace in the range [start_, stop_] and return the result.
    float IntegratePulse(const size_t &start_=0, const size_t &stop_=0);
    
    /// Perform CFD analysis on the waveform.
    float AnalyzeCFD(const float &F_=0.5, const size_t &D_=1, const size_t &L_=1);
    
    /// Clear all variables and clear the trace vector and arrays.
    void Clear();
};

#endif
