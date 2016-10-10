#include <cmath>
#include <string.h>

#include "XiaData.hpp"

/////////////////////////////////////////////////////////////////////
// XiaData
/////////////////////////////////////////////////////////////////////

/// Default constructor.
XiaData::XiaData(){
	adcTrace = NULL;
	qdcValue = NULL;
	clear();
}

/// Constructor from a pointer to another XiaData.
XiaData::XiaData(XiaData *other_){
	adcTrace = NULL;
	qdcValue = NULL;
	clear();

	energy = other_->energy; 
	time = other_->time;

	modNum = other_->modNum;
	chanNum = other_->chanNum;
	cfdTime = other_->cfdTime;
	eventTimeLo = other_->eventTimeLo;
	eventTimeHi = other_->eventTimeHi;

	virtualChannel = other_->virtualChannel;
	pileupBit = other_->pileupBit;
	saturatedBit = other_->saturatedBit;
	cfdForceTrig = other_->cfdForceTrig; 
	cfdTrigSource = other_->cfdTrigSource; 

	// Copy the ADC trace, if enabled.
	if(other_->traceLength > 0)
		copyTrace((char *)other_->adcTrace, other_->traceLength);

	// Copy the onboard QDCs, if enabled.
	if(other_->numQdcs > 0)
		copyQDCs((char *)other_->qdcValue, other_->numQdcs);
}

XiaData::~XiaData(){
	clearTrace();
	clearQDCs();
}

/// Fill the trace by reading from a character array.
void XiaData::copyTrace(char *ptr_, const unsigned short &size_){
	if(size_ == 0) return;
	else if(adcTrace == NULL || size_ != traceLength){
		clearTrace();
		traceLength = size_;
		adcTrace = new unsigned short[traceLength];
	}
	memcpy((char *)adcTrace, ptr_, traceLength*2);
}

/// Fill the QDC array by reading a character array.
void XiaData::copyQDCs(char *ptr_, const unsigned short &size_){
	if(size_ == 0) return;
	else if(qdcValue == NULL || size_ != numQdcs){
		clearQDCs();
		numQdcs = size_;
		qdcValue = new unsigned int[numQdcs];
	}
	memcpy((char *)qdcValue, ptr_, numQdcs*4);
}

void XiaData::clear(){
	energy = 0.0; 
	time = 0.0;
	
	modNum = 0;
	chanNum = 0;
	cfdTime = 0;
	eventTimeLo = 0;
	eventTimeHi = 0;

	virtualChannel = false;
	pileupBit = false;
	saturatedBit = false;
	cfdForceTrig = false; 
	cfdTrigSource = false; 
	
	clearTrace();
	clearQDCs();
}

/// Delete the trace.
void XiaData::clearTrace(){
	if(adcTrace != NULL)
		delete[] adcTrace;
	
	traceLength = 0;
	adcTrace = NULL;
}

/// Delete the QDC array.
void XiaData::clearQDCs(){
	if(qdcValue != NULL)
		delete[] qdcValue;
	
	numQdcs = 0;
	qdcValue = NULL;
}

/// Get the size of the XiaData event when written to disk by ::writeRaw (in 4-byte words).
size_t XiaData::getEventLength(){
	size_t eventLength = 4;
	if(numQdcs > 0) eventLength += numQdcs; // Account for the onboard QDCs.
	if(traceLength > 0) eventLength += traceLength/2; // Account for the ADC trace.
	return eventLength;
}

/// Print event information to the screen.
void XiaData::print(){
	std::cout << " energy:      " << this->energy << std::endl;
	std::cout << " time:        " << this->time << std::endl;
	std::cout << " traceLength: " << this->traceLength << std::endl;
	std::cout << " numQdcs:     " << this->numQdcs << std::endl;
	std::cout << " slotNum:     " << this->slotNum << std::endl;
	std::cout << " modNum:      " << this->modNum << std::endl;
	std::cout << " chanNum:     " << this->chanNum << std::endl;
	std::cout << " cfdTime:     " << this->cfdTime << std::endl;
	std::cout << " eventTimeLo: " << this->eventTimeLo << std::endl;
	std::cout << " eventTimeHi: " << this->eventTimeHi << std::endl;
}

/** Responsible for decoding individual pixie events from a binary input file.
  * \param[in]  buf         Pointer to an array of unsigned ints containing raw event data.
  * \param[in]  module      The current module number being scanned.
  * \param[out] bufferIndex The current index in the module buffer.
  * \return Only false currently. This method is only a stub.
  */
bool XiaData::readEventRevD(unsigned int *buf, unsigned int &bufferIndex, unsigned int module/*=9999*/){
	return false;
}

/** Responsible for decoding individual pixie events from a binary input file.
  * \param[in]  buf         Pointer to an array of unsigned ints containing raw event data.
  * \param[in]  module      The current module number being scanned.
  * \param[out] bufferIndex The current index in the module buffer.
  * \return True if the event was successfully read, or false otherwise.
  */
bool XiaData::readEventRevF(unsigned int *buf, unsigned int &bufferIndex, unsigned int module/*=9999*/){	
	// Multiplier for high bits of 48-bit time
	static const double HIGH_MULT = pow(2., 32.); 

	// Decoding event data... see pixie16app.c
	// buf points to the start of channel data
	chanNum        =  (buf[bufferIndex] & 0x0000000F);
	slotNum        =  (buf[bufferIndex] & 0x000000F0) >> 4;
	crateNum       =  (buf[bufferIndex] & 0x00000F00) >> 8;
	headerLength   =  (buf[bufferIndex] & 0x0001F000) >> 12;
	eventLength    =  (buf[bufferIndex] & 0x1FFE0000) >> 17;
	virtualChannel = ((buf[bufferIndex] & 0x20000000) != 0);
	saturatedBit   = ((buf[bufferIndex] & 0x40000000) != 0);
	pileupBit      = ((buf[bufferIndex] & 0x80000000) != 0);	

	eventTimeLo =  buf[bufferIndex + 1];
	eventTimeHi =  buf[bufferIndex + 2] & 0x0000FFFF;
	cfdTime     = (buf[bufferIndex + 2] & 0xFFFF0000) >> 16;
	energy      =  buf[bufferIndex + 3] & 0x0000FFFF;
	traceLength = (buf[bufferIndex + 3] & 0xFFFF0000) >> 16;

	// Handle saturated filter energy.
	if(saturatedBit){ energy = 16383; }
		
	// Calculate the 48-bit trigger time.	
	time = eventTimeHi * HIGH_MULT + eventTimeLo;

	if(module == 9999) modNum = slotNum;
	else modNum = module;

	// Handle multiple crates.
	modNum += 100 * crateNum;

	// Rev. D header lengths not clearly defined in pixie16app_defs
	//! magic numbers here for now
	if(headerLength == 1){
		// this is a manual statistics block inserted by the poll program
		/*stats.DoStatisticsBlock(&buf[bufferIndex + 1], modNum);
		numEvents = -10;*/
		
		// Advance to next event.
		bufferIndex += eventLength;
		return false;
	}
	
	// Check that the header length is valid.
	if(headerLength != 4 && headerLength != 8 && headerLength != 12 && headerLength != 16){
		std::cout << "ReadEventRevF: Unexpected header length: " << headerLength << std::endl;
		std::cout << "ReadEventRevF:  Module " << modNum << std::endl;
		std::cout << "ReadEventRevF:  CHAN:SLOT:CRATE " << chanNum << ":" << slotNum << ":" << crateNum << std::endl;
		
		// Check for zero event length.
		if(eventLength == 0){
			std::cout << "ReadEventRevF: ERROR! Encountered zero event length. Moving ahead one word.\n";
			bufferIndex += 1;
			return false;
		}
		
		// Advance to next event.
		bufferIndex += eventLength;
		return false;
	}

	// One last check on the event length.
	if( traceLength / 2 + headerLength != eventLength ){
		std::cout << "ReadEventRevF: Bad event length (" << eventLength << ") does not correspond with length of header (";
		std::cout << headerLength << ") and length of trace (" << traceLength << ")" << std::endl;
		
		// Advance to next event.
		bufferIndex += eventLength;
		return false;
	}

	// Move the buffer index past the header.
	bufferIndex += 4;

	if(headerLength == 8 || headerLength == 16){
		// Skip the onboard partial sums for now 
		// trailing, leading, gap, baseline
		bufferIndex += 4;
	}

	if(headerLength >= 12){ // Copy the QDCs.
		copyQDCs((char *)&buf[bufferIndex], 8);
		bufferIndex += 8;
	}

	/*if(currentEvt->virtualChannel){
		DetectorLibrary* modChan = DetectorLibrary::get();

		currentEvt->modNum += modChan->GetPhysicalModules();
		if(modChan->at(modNum, chanNum).HasTag("construct_trace")){
			lastVirtualChannel = currentEvt;
		}
	}*/

	// Check if trace data follows the channel header
	if( traceLength > 0 ){
		/*if(currentEvt->saturatedBit)
			currentEvt->trace.SetValue("saturation", 1);*/

		/*if( lastVirtualChannel != NULL && lastVirtualChannel->traceLength == 0 ){		
			lastVirtualChannel->assign(0);
		}*/
		// Read the trace data (2-bytes per sample, i.e. 2 samples per word)
		copyTrace((char *)&buf[bufferIndex], traceLength);
		bufferIndex += (traceLength / 2);
	}
	
	return true;
}

/** Write a pixie style event to a binary output file. Output data may
  * be written to both an ofstream and a character array. One of the
  * pointers must not be NULL.
  * 
  * \param[in] file_ Pointer to an ofstream output binary file.
  * \param[in] array_ Pointer to a character array into which data will be written.
  * \return The number of bytes written to the file upon success and -1 otherwise.
  */
int XiaData::writeEventRevF(std::ofstream *file_, char *array_){
	if((!file_ && !array_) || (file_ && !file_->good())) return -1;

	unsigned int crateNum = 0x0; // Fixed value for now.
	unsigned int chanIdentifier = 0xFFFFFFFF;
	unsigned int eventTimeHiWord = 0xFFFFFFFF;
	unsigned int eventEnergyWord = 0xFFFFFFFF;

	unsigned int eventLength = (unsigned int)getEventLength();
	unsigned int headLength = eventLength - (unsigned int)traceLength/2;
	
	// Build up the channel identifier.
	chanIdentifier &= ~(0x0000000F & (chanNum));              // Pixie channel number
	chanIdentifier &= ~(0x000000F0 & (modNum << 4));          // Pixie module number (NOT the slot number)
	chanIdentifier &= ~(0x00000F00 & (crateNum << 8));        // Crate number
	chanIdentifier &= ~(0x0001F000 & (headLength << 12));     // Header length
	chanIdentifier &= ~(0x1FFE0000 & (eventLength << 17));    // Event length
	chanIdentifier &= ~(0x20000000 & (virtualChannel << 29)); // Virtual channel bit
	chanIdentifier &= ~(0x40000000 & (saturatedBit << 30));   // Saturated channel bit
	chanIdentifier &= ~(0x80000000 & (pileupBit << 31));      // Pileup bit
	chanIdentifier = ~chanIdentifier;
	
	// Build up the high event time and CFD time.
	eventTimeHiWord &= ~(0x0000FFFF & (eventTimeHi));
	eventTimeHiWord &= ~(0xFFFF0000 & (cfdTime << 16));
	eventTimeHiWord = ~eventTimeHiWord;
	
	// Build up the event energy.
	eventEnergyWord &= ~(0x0000FFFF & (energy));
	eventEnergyWord &= ~(0xFFFF0000 & (traceLength << 16));
	eventEnergyWord = ~eventEnergyWord;
	
	if(file_){
		// Write data to the output file.
		file_->write((char *)&chanIdentifier, 4);
		file_->write((char *)&eventTimeLo, 4);
		file_->write((char *)&eventTimeHiWord, 4);
		file_->write((char *)&eventEnergyWord, 4);
	}
	
	if(array_){
		// Write data to the character array.
		memcpy(array_, (char *)&chanIdentifier, 4);
		memcpy(&array_[4], (char *)&eventTimeLo, 4);
		memcpy(&array_[8], (char *)&eventTimeHiWord, 4);
		memcpy(&array_[12], (char *)&eventEnergyWord, 4);
	}

	int numBytes = 16;

	// Write the onbard QDCs, if enabled.
	if(numQdcs > 0){
		if(file_) file_->write((char *)qdcValue, numQdcs*4);
		if(array_) memcpy(&array_[numBytes], (char *)qdcValue, numQdcs*4);
		numBytes += numQdcs*4;
	}

	// Write the ADC trace, if enabled.
	if(traceLength != 0){ // Write the trace.
		if(file_) file_->write((char *)adcTrace, traceLength*2);
		if(array_) memcpy(&array_[numBytes], (char *)adcTrace, traceLength*2);
		numBytes += traceLength*2;
	}
	
	return numBytes;
}

/////////////////////////////////////////////////////////////////////
// ChannelEvent
/////////////////////////////////////////////////////////////////////

/// Default constructor.
ChannelEvent::ChannelEvent(){
	Clear();
}

/// Constructor from a XiaData. ChannelEvent will take ownership of the XiaData.
ChannelEvent::ChannelEvent(XiaData *event_) : XiaData(event_) {
	Clear();
}

ChannelEvent::~ChannelEvent(){
	if(cfdvals) delete[] cfdvals;
}

float ChannelEvent::ComputeBaseline(){
	if(traceLength == 0){ return -9999; }
	if(baseline > 0){ return baseline; }

	// Find the baseline.
	baseline = 0.0;
	size_t sample_size = (15 <= traceLength ? 15:traceLength);
	for(size_t i = 0; i < sample_size; i++){
		baseline += adcTrace[i];
	}
	baseline = float(baseline)/sample_size;
	
	// Calculate the standard deviation.
	stddev = 0.0;
	for(size_t i = 0; i < sample_size; i++){
		stddev += (adcTrace[i] - baseline)*(adcTrace[i] - baseline);
	}
	stddev = std::sqrt((1.0/sample_size) * stddev);
	
	// Find the maximum value and the maximum bin.
	maximum = -9999.0;
	for(size_t i = 0; i < traceLength; i++){
		if(adcTrace[i]-baseline > maximum){ 
			maximum = adcTrace[i]-baseline;
			max_index = i;
		}
	}
	
	return baseline;
}

float ChannelEvent::IntegratePulse(const size_t &start_/*=0*/, const size_t &stop_/*=0*/){
	if(traceLength == 0 || baseline < 0.0){ return -9999; }
	
	size_t stop = (stop_ == 0?traceLength:stop_);
	
	qdc = 0.0;
	for(size_t i = start_+1; i < stop; i++){ // Integrate using trapezoidal rule.
		qdc += 0.5*(adcTrace[i-1] + adcTrace[i]) - baseline;
	}

	return qdc;
}

/// Perform CFD analysis on the waveform.
float ChannelEvent::AnalyzeCFD(const float &F_/*=0.5*/, const size_t &D_/*=1*/, const size_t &L_/*=1*/){
	if(traceLength == 0 || baseline < 0){ return -9999; }
	if(!cfdvals)
		cfdvals = new float[traceLength];
	
	float cfdMinimum = 9999;
	size_t cfdMinIndex = 0;
	
	phase = -9999;

	// Compute the cfd waveform.
	for(size_t cfdIndex = 0; cfdIndex < traceLength; ++cfdIndex){
		cfdvals[cfdIndex] = 0.0;
		if(cfdIndex >= L_ + D_ - 1){
			for(size_t i = 0; i < L_; i++)
				cfdvals[cfdIndex] += F_ * (adcTrace[cfdIndex - i]-baseline) - (adcTrace[cfdIndex - i - D_]-baseline);
		}
		if(cfdvals[cfdIndex] < cfdMinimum){
			cfdMinimum = cfdvals[cfdIndex];
			cfdMinIndex = cfdIndex;
		}
	}

	// Find the zero-crossing.
	if(cfdMinIndex > 0){
		// Find the zero-crossing.
		for(size_t cfdIndex = cfdMinIndex-1; cfdIndex >= 0; cfdIndex--){
			if(cfdvals[cfdIndex] >= 0.0 && cfdvals[cfdIndex+1] < 0.0){
				phase = cfdIndex - cfdvals[cfdIndex]/(cfdvals[cfdIndex+1]-cfdvals[cfdIndex]);
				break;
			}
		}
	}

	return phase;
}

void ChannelEvent::Clear(){
	hiresTime = 0.0;

	phase = -9999;
	maximum = -9999;
	baseline = -9999;
	stddev = -9999;
	qdc = -9999;
	max_index = 0;

	valid_chan = false;
	ignore = false;
	
	cfdvals = NULL;
	
	if(cfdvals) 
		delete[] cfdvals;
}

/** Responsible for decoding ChannelEvents from a binary input file.
  * \param[in]  buf         Pointer to an array of unsigned ints containing raw event data.
  * \param[in]  modNum     The current module number being scanned.
  * \param[out] bufferIndex The current index in the module buffer.
  * \return True if the event was successfully read, or false otherwise.
  */
bool ChannelEvent::readEvent(unsigned int *buf, unsigned int &bufferIndex){
	return false;
}

/** Write a ChannelEvent to a binary output file. Output data may
  * be written to both an ofstream and a character array. One of the
  * pointers must not be NULL.
  * 
  * \param[in] file_ Pointer to an ofstream output binary file.
  * \param[in] array_ Pointer to a character array into which data will be written.
  * \return The number of bytes written to the file upon success and -1 otherwise.
  */
int ChannelEvent::writeEvent(std::ofstream *file_, char *array_){
	return -1;
}
