/** \file Unpacker.hpp
 * \brief A class to handle the unpacking of UTK/ORNL style pixie16 data spills.
 *
 * This class is intended to be used as a replacement of pixiestd.cpp from Stan
 * Paulauskas's pixie_scan. The majority of function names and arguments are
 * preserved as much as possible while allowing for more standardized unpacking
 * of pixie16 data.
 * CRT
 *
 * \author C. R. Thornsberry
 * \date Feb. 12th, 2016
 */
#ifndef UNPACKER_HPP
#define UNPACKER_HPP

#include <deque>
#include <vector>
#include <string>

#ifndef MAX_PIXIE_MOD
#define MAX_PIXIE_MOD 12
#endif
#ifndef MAX_PIXIE_CHAN
#define MAX_PIXIE_CHAN 15
#endif

class XiaData;
class ScanMain;
class ScanInterface;

class Unpacker{
  public:
  	/// Default constructor.
	Unpacker();

	/// Destructor.
	virtual ~Unpacker();

	/// Return the maximum module read from the input file.
	size_t GetMaxModule(){ return eventList.size(); }

	/// Return the number of raw events read from the file.
	unsigned int GetNumRawEvents(){ return numRawEvt; }
	
	/// Return the width of the raw event window in pixie16 clock ticks.
	double GetEventWidth(){ return eventWidth; }

	/// Return the delay of events in pixie16 clock ticks.
	double GetEventDelay(){ return eventDelay; }

	/** Get the method to use for raw event building.
	  *  (0) : Untriggered, positive time. Raw event window opens toward positive time for any detected signal (default).
	  *  (1) : Untriggered, negative time. Raw event window opens toward negative time for any detected signal.
	  *  (2) : Triggered, positive time. Raw event window opens toward positive time for a specified start channel.
	  *  (3) : Triggered, negative time. Raw event window opens toward negative time for a specified start channel.
	  * \return The raw event building method.
	  */
	int GetRawEventMode(){ return rawEventMode; }
	
	/// Return the time of the first fired channel event.
	double GetFirstTime(){ return firstTime; }
	
	/// Get the start time of the current raw event.
	double GetEventStartTime(){ return eventStartTime; }
	
	/// Get the stop time of the current raw event.
	double GetEventStopTime(){ return eventStartTime+eventWidth; }

	/// Get the time of the first xia event in the raw event.
	double GetRealStartTime(){ return realStartTime; }
	
	/// Get the time of the last xia event in the raw event.
	double GetRealStopTime(){ return realStopTime; }

	/// Return true if the scan is running and false otherwise.
	bool IsRunning(){ return running; }

	/// Toggle debug mode on / off.
	bool SetDebugMode(bool state_=true){ return (debug_mode = state_); }
	
	/// Set the width of events in pixie16 clock ticks.
	double SetEventWidth(double width_){ return (eventWidth = width_); }

	/// Set the delay of events in pixie16 clock ticks.
	double SetEventDelay(double delay_){ return (eventDelay = delay_); }

	/** Set the method to use for raw event building.
	  *  (0) : Untriggered, positive time. Raw event window opens toward positive time for any detected signal (default).
	  *  (1) : Untriggered, negative time. Raw event window opens toward negative time for any detected signal.
	  *  (2) : Triggered, positive time. Raw event window opens toward positive time for a specified start channel.
	  *  (3) : Triggered, negative time. Raw event window opens toward negative time for a specified start channel.
	  * \param[in]  mode_ Set the raw event building method.
	  * \return The raw event building method upon success and -1 upon failure.
	  */
	int SetRawEventMode(const int &mode_){ return ((mode_ >= 0 || mode_ <= 3) ? (rawEventMode = mode_) : -1); }
	
	/// Set the address of the scan interface used for file operations.
	ScanInterface *SetInterface(ScanInterface *interface_){ return (interface = interface_); }
	
	/** ReadSpill is responsible for constructing a list of pixie16 events from
	  * a raw data spill. This method performs sanity checks on the spill and
	  * calls ReadBuffer in order to construct the event list.
	  * \param[in]  data       Pointer to an array of unsigned ints containing the spill data.
	  * \param[in]  nWords     The number of words in the array.
	  * \param[in]  is_verbose Toggle the verbosity flag on/off.
	  * \return True if the spill was read successfully and false otherwise.
	  */	
	bool ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose=true);
	
	/** ReadRawEvent assumes that the events in the incoming data are already grouped
	  * into a raw event. This method performs sanity checks on the raw event and calls
	  * ReadBuffer in order to construct the event list.
	  * \param[in]  data       Pointer to an array of unsigned ints containing the spill data.
	  * \param[in]  nWords     The number of words in the array.
	  * \param[in]  is_verbose Toggle the verbosity flag on/off.
	  * \return True if the spill was read successfully and false otherwise.
	  */	
	bool ReadRawEvent(unsigned int *data, unsigned int nWords, bool is_verbose=true);
	
	/** Write all recorded channel counts to a file.
	  * \return Nothing.
	  */
	void Write();
	
	/** Stop the scan. Unused by default.
	  * \return Nothing.
	  */
	void Stop(){ running = false; }
	
	/** Run the scan. Unused by default.
	  * \return Nothing.
	  */
	void Run(){ running = true; }

	/** Clear event list and raw event.
	  * \return Nothing.
	  */
	void Clear();
	
  protected:
	double eventWidth; /// The width of the raw event window in pixie clock ticks (8 ns).
	double eventDelay; /// The delay of the raw event window from a start signal in pixie clock ticks (8 ns).
	
	bool debug_mode; /// True if debug mode is set.
	bool running; /// True if the scan is running.

	std::vector<std::deque<XiaData*> > eventList; /// The list of all events in a spill.
	std::deque<XiaData*> startList; /// The list of all start events in a spill.
	std::deque<XiaData*> rawEvent; /// The list of all events in the event window.

	ScanInterface *interface; /// Pointer to an object derived from ScanInterface.

	/** Return a pointer to a new XiaData channel event.
	  * \return A pointer to a new XiaData.
	  */
	virtual XiaData *GetNewEvent();

	/** Process all events in the event list.
	  * \param[in]  addr_ Pointer to a ScanInterface object. Unused by default.
	  * \return Nothing.
	  */
	virtual void ProcessRawEvent(ScanInterface *addr_=NULL);
	
	/** Add an event to generic statistics output.
	  * \param[in]  event_ Pointer to the current XIA event. Unused by default.
	  * \param[in]  addr_  Pointer to a ScanInterface object. Unused by default.
	  * \return Nothing.
	  */
	virtual void RawStats(XiaData *event_, ScanInterface *addr_=NULL){  }
	
	/** Called form ReadSpill. Scan the current spill and construct a list of
	  * events which fired by obtaining the module, channel, trace, etc. of the
	  * timestamped event. This method will construct the event list for
	  * later processing.
	  * \param[in]  buf    Pointer to an array of unsigned ints containing raw module data.
	  * \return The number of XiaData events read from the module buffer.
	  */	
	int ReadSpillModule(unsigned int *buf);
	
  private:
	unsigned int TOTALREAD; /// Maximum number of data words to read.
	unsigned int maxWords; /// Maximum number of data words for revision D.
	unsigned int numRawEvt; /// The total count of raw events read from file.
	
	unsigned int channel_counts[MAX_PIXIE_MOD+1][MAX_PIXIE_CHAN+1]; /// Counters for each channel in each module.
	
	double firstTime; /// The first recorded event time.
	double eventStartTime; /// The start time of the current raw event.

	double realStartTime; /// The time of the first xia event in the raw event.
	double realStopTime; /// The time of the last xia event in the raw event.

	int rawEventMode; /// Set the raw event building method to use.

	unsigned short startMod; /// Start module to use for triggered raw event building.
	unsigned short startChan; /// Start channel to use for triggered raw event building.

	/** Scan the event list and sort it by timestamp.
	  * \return Nothing.
	  */
	void TimeSort();

	/** Scan the time sorted event list and package the events into a raw
	  * event with a size governed by the event width.
	  * \return True if the event list is not empty and false otherwise.
	  */
	bool BuildRawEventA();
	
	/** Scan the time sorted event list and package the events into a raw
	  * event with a size governed by the event width.
	  * \return True if the event list is not empty and false otherwise.
	  */
	bool BuildRawEventB();
	
	/** Push an event into the event list.
	  * \param[in]  event_ The XiaData to push onto the back of the event list.
	  * \return True if the XiaData's module number is valid and false otherwise.
	  */
	bool AddEvent(XiaData *event_);
	
	/** Clear all events in the spill event list. WARNING! This method will delete all events in the
	  * event list. This could cause seg faults if the events are used elsewhere.
	  * \return Nothing.
	  */	
	void ClearEventList();

	/** Clear all events in the raw event list. WARNING! This method will delete all events in the
	  * event list. This could cause seg faults if the events are used elsewhere.
	  * \return Nothing.
	  */	
	void ClearRawEvent();
	
	/** Get the minimum channel time from the event list.
	  * \param[out] time The minimum time from the event list in system clock ticks.
	  * \return True if the event list is not empty and false otherwise.
	  */
	bool GetFirstTime(double &time);
	
	/** Check whether or not the eventList is empty.
	  * \return True if the eventList is empty, and false otherwise.
	  */
	bool IsEmpty();
};

#endif
