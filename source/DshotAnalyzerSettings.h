#ifndef DSHOT_ANALYZER_SETTINGS
#define DSHOT_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DshotAnalyzerSettings : public AnalyzerSettings
{
public:
	DshotAnalyzerSettings();
	virtual ~DshotAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	Channel mInputChannel;
	U32 mDshotRate;

protected:
	std::unique_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mDshotRateInterface;
};

#endif //DSHOT_ANALYZER_SETTINGS
