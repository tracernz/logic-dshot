#ifndef DSHOT_SIMULATION_DATA_GENERATOR
#define DSHOT_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class DshotAnalyzerSettings;

class DshotSimulationDataGenerator
{
public:
	DshotSimulationDataGenerator();
	~DshotSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, DshotAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	DshotAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateChannelUpdate();

	double mChannelRadians;
	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //DSHOT_SIMULATION_DATA_GENERATOR