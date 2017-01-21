#include "DshotSimulationDataGenerator.h"
#include "DshotAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

#include <cmath>
#include <stdint.h>

DshotSimulationDataGenerator::DshotSimulationDataGenerator() :
	mChannelRadians(0)
{
}

DshotSimulationDataGenerator::~DshotSimulationDataGenerator()
{
}

void DshotSimulationDataGenerator::Initialize( U32 simulation_sample_rate, DshotAnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mSerialSimulationData.SetChannel( mSettings->mInputChannel );
	mSerialSimulationData.SetSampleRate( simulation_sample_rate );
	mSerialSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 DshotSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while(mSerialSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested)
		CreateChannelUpdate();

	*simulation_channel = &mSerialSimulationData;
	return 1;
}

void DshotSimulationDataGenerator::CreateChannelUpdate()
{
	uint32_t samples_per_bit = mSimulationSampleRateHz / (mSettings->mDshotRate * 1000);

	uint16_t channel = 0;

	uint16_t chan_val = (2047.0 / 2.0) + (2047.0 / 2.0) * std::sin(mChannelRadians);
	bool telem = chan_val > 2045;
	chan_val <<= 5;
	uint8_t crc = 	((chan_val >> 4 ) & 0xf) ^
					((chan_val >> 8 ) & 0xf) ^
					((chan_val >> 12) & 0xf);

	channel |= chan_val;
	channel |= (telem & 1) << 4;
	channel |= (crc & 0xf);
	mChannelRadians += 0.1; // TODO: set frequency

	mSerialSimulationData.TransitionIfNeeded(BIT_LOW);
	mSerialSimulationData.Advance(samples_per_bit * 10);

	U32 width = 1e6 / mSettings->mDshotRate;
	uint32_t dshot_true = 0.75 * width;
	uint32_t dshot_false = 0.375 * dshot_true;
	for (int i = sizeof(channel) * 8 - 1; i >= 0; i--) {
		uint32_t high = (channel & 1 << i) ? dshot_true : dshot_false;
		uint32_t low = width - high;
		mSerialSimulationData.TransitionIfNeeded(BIT_HIGH);
		mSerialSimulationData.Advance(static_cast<double>(mSimulationSampleRateHz) * (high * 1e-9));
		mSerialSimulationData.TransitionIfNeeded(BIT_LOW);
		mSerialSimulationData.Advance(static_cast<double>(mSimulationSampleRateHz) * (low * 1e-9));
	}

	//lets pad the end a bit for the stop bit:
	mSerialSimulationData.Advance(samples_per_bit);
}
