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
	if(other_->traceLength > 0)
		copyTrace((char *)other_->adcTrace, other_->traceLength);

	if(other_->numQdcs > 0)
		copyQDCs((char *)other_->qdcValue, other_->numQdcs);

	energy = other_->energy; 
	time = other_->time;

	modNum = other_->modNum;
	chanNum = other_->chanNum;
	trigTime = other_->trigTime;
	cfdTime = other_->cfdTime;
	eventTimeLo = other_->eventTimeLo;
	eventTimeHi = other_->eventTimeHi;
	eventTime = other_->eventTime;

	virtualChannel = other_->virtualChannel;
	pileupBit = other_->pileupBit;
	saturatedBit = other_->saturatedBit;
	cfdForceTrig = other_->cfdForceTrig; 
	cfdTrigSource = other_->cfdTrigSource; 
}

XiaData::~XiaData(){
	clearTrace();
	clearQDCs();
}

void XiaData::reserve(const size_t &size_){
	if(size_ == 0) return;
	else if(traceLength != 0){
		if(size_ == traceLength)
			return;
		clearTrace();
	}
	traceLength = size_;
	adcTrace = new unsigned short[traceLength];
}

void XiaData::assign(const unsigned short &input_){
	if(traceLength == 0) return;
	for(size_t index = 0; index < traceLength; index++)
		adcTrace[index] = input_;
}

/// Fill the trace by reading from a character array.
void XiaData::copyTrace(char *ptr_){
	if(traceLength == 0) return;
	memcpy((char *)adcTrace, ptr_, traceLength*2);
}

/// Fill the trace by reading from a character array.
void XiaData::copyTrace(char *ptr_, const size_t &size_){
	reserve(size_);
	copyTrace(ptr_);
}

/// Fill the QDC array by reading a character array.
void XiaData::copyQDCs(char *ptr_, const size_t &size_){
	if(size_ == 0) return;
	else if(numQdcs == 0 || size_ != numQdcs){
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
	trigTime = 0;
	cfdTime = 0;
	eventTimeLo = 0;
	eventTimeHi = 0;
	eventTime = 0.0;

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

/// Get the size of the XiaData event when written to disk by ::writeRaw (in bytes).
size_t XiaData::getEventLength(){
	size_t eventLength = 4;
	if(numQdcs > 0) eventLength += numQdcs; // Account for the onboard QDCs.
	if(traceLength > 0) eventLength += traceLength/2; // Account for the ADC trace.
	return eventLength;
}

/// Write a pixie style event to a binary output file.
int XiaData::writeRaw(std::ofstream &file_){
/*	if(!file_.good()) return -1;

	unsigned int chanIdentifier = 0xFFFFFFFF;
	unsigned int eventTimeHiWord = 0xFFFFFFFF;
	unsigned long long eventTimeLL;
	
	// Build up the channel identifier.
	
	// Build up the event time.
	memcpy((char *)&eventTimeLL, (char *)&eventTime, 8);
	eventTimeLo = eventTimeLL >> 32;
	eventTimeHi = (eventTimeLL & 0xFFFF0000);
	int whatever    = (eventTimeLL & 0x0000FFFF);
	
	eventTimeHiWord &= eventTimeHi;
	eventTimeHiWord &= 
	
	// Build up the CFD time.
	
	// Build up the event energy.
	
	
	// Write data to the output file.
	file_.write((char *)&chanIdentifier, 4);
	file_.write((char *)&eventTimeLo, 4);
	file_.write((char *)&eventTimeHi, 4);



	int numHalfWords = 8;

	if(traceLength != 0){ // Write the trace.
		file_.write((char *)adcTrace, traceLength*2);
		numHalfWords += (int)traceLength;
	}
	
	return (numHalfWords / 2);*/
	return -1;
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
