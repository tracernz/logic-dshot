#include "DshotAnalyzer.h"
#include "DshotAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <AnalyzerHelpers.h>

#include <stdint.h>
#include <cstdio>

DshotAnalyzer::DshotAnalyzer()
:	Analyzer(),  
	mSettings( new DshotAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

DshotAnalyzer::~DshotAnalyzer()
{
	KillThread();
}

double DshotAnalyzer::proportionOfBit(U32 width)
{
	return static_cast<double>(width) / mSamplesPerBit;
}

void DshotAnalyzer::WorkerThread()
{
	mResults.reset( new DshotAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );

	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
	mSamplesPerBit = mSampleRateHz / (mSettings->mDshotRate * 1000);

	if( mSerial->GetBitState() == BIT_HIGH )
		mSerial->AdvanceToNextEdge();

	uint32_t width = 0;

	for (;;) {
		uint16_t data = 0;
		uint64_t starting_sample = 0;
		int i;
		for (i = sizeof(data) * 8 - 1; i >= 0; i--) {
			mSerial->AdvanceToNextEdge(); //rising edge of first bit
			uint64_t rising_sample = mSerial->GetSampleNumber();
			if (!starting_sample)
				starting_sample = rising_sample;
			mSerial->AdvanceToNextEdge();
			uint64_t falling_sample = mSerial->GetSampleNumber();

			width = falling_sample - rising_sample;
			bool set = proportionOfBit(width) > 0.5;
			// check if low pulse is too long / next bit is too far away
			bool error = i > 0 && proportionOfBit(mSerial->GetSampleOfNextEdge() - falling_sample) > 1.5;
			// check if high pulse is too short
			error |= proportionOfBit(width) < 0.2;

			if (set)
				data |= 1 << i;

			AnalyzerResults::MarkerType marker;
			if (error) {
				marker = AnalyzerResults::ErrorX;
			} else if (i == 4) { // telem request
				if (set)
					marker = AnalyzerResults::Start;
				else
					marker = AnalyzerResults::Stop;
			} else { // channel bits or crc
				if (set)
					marker = AnalyzerResults::One;
				else
					marker = AnalyzerResults::Zero;
			}
			mResults->AddMarker(rising_sample + width / 2, marker, mSettings->mInputChannel);

			if (error)
				break;
		}

		if (i >= 0) { // message ended early / bit was errored
			mResults->CommitResults();
			continue;
		}

		uint16_t chan = data & 0xffe0;
		uint8_t crc =	((chan >> 4 ) & 0xf) ^
						((chan >> 8 ) & 0xf) ^
						((chan >> 12) & 0xf);
		bool crcok = (data & 0xf) == crc;
		chan >>= 5;

		mSerial->Advance(mSamplesPerBit - width); // end of low pulse

		if (!crcok)
			mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mInputChannel);
		
		//we have a byte to save. 
		Frame frame;
		frame.mData1 = chan;
		frame.mFlags = (crcok ? 0 : DISPLAY_AS_ERROR_FLAG) | ((data & 0x10) > 0); // error flag | telem request
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

		mResults->AddFrame(frame);
		mResults->CommitResults();
		ReportProgress(frame.mEndingSampleInclusive);
	}
}

bool DshotAnalyzer::NeedsRerun()
{
	return false;
}

U32 DshotAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 DshotAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mDshotRate * 4000;
}

const char* DshotAnalyzer::GetAnalyzerName() const
{
	return "Dshot";
}

const char* GetAnalyzerName()
{
	return "Dshot";
}

Analyzer* CreateAnalyzer()
{
	return new DshotAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}