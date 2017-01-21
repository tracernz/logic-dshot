#include "DshotAnalyzer.h"
#include "DshotAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#include <stdint.h>

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

void DshotAnalyzer::WorkerThread()
{
	mResults.reset( new DshotAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );

	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

	if( mSerial->GetBitState() == BIT_HIGH )
		mSerial->AdvanceToNextEdge();

	U32 samples_per_bit = mSampleRateHz / (mSettings->mDshotRate * 1000);

	uint64_t width = 0;

	for (;;) {
		uint16_t data = 0;
		uint64_t starting_sample = 0;

		for (int i = sizeof(data) * 8 - 1; i >= 0; i--) {
			mSerial->AdvanceToNextEdge(); //rising edge of first bit
			uint64_t rising_sample = mSerial->GetSampleNumber();
			if (!starting_sample)
				starting_sample = rising_sample;
			mSerial->AdvanceToNextEdge();
			uint64_t falling_sample = mSerial->GetSampleNumber();

			width = falling_sample - rising_sample;

			bool set = (static_cast<double>(width) / samples_per_bit) > 0.5;

			if (set)
				data |= 1 << i;

			AnalyzerResults::MarkerType marker;
			if (i == 4) { // telem request
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
		}

		uint16_t chan = data & 0xffe0;
		uint8_t crc =	((chan >> 4 ) & 0xf) ^
						((chan >> 8 ) & 0xf) ^
						((chan >> 12) & 0xf);
		bool crcok = (data & 0xf) == crc;
		chan >>= 5;

		mSerial->Advance(samples_per_bit - width); // end of low pulse

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