#include "DshotAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


DshotAnalyzerSettings::DshotAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mDshotRate( 300 )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Serial", "Standard Dshot" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mDshotRateInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mDshotRateInterface->SetTitleAndTooltip( "Bit Rate (kbits/s)",  "Specify the bit rate in kbits per second." );
	mDshotRateInterface->ClearNumbers();
	mDshotRateInterface->AddNumber(150, "Dshot150", "150 kbit/s");
	mDshotRateInterface->AddNumber(300, "Dshot300", "300 kbit/s");
	mDshotRateInterface->AddNumber(600, "Dshot600", "600 kbit/s");
	mDshotRateInterface->AddNumber(1200, "Dshot1200", "1200 kbit/s");
	mDshotRateInterface->SetNumber(mDshotRate);

	AddInterface( mInputChannelInterface.get() );
	AddInterface( mDshotRateInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Serial", false );
}

DshotAnalyzerSettings::~DshotAnalyzerSettings()
{
}

bool DshotAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mDshotRate = mDshotRateInterface->GetNumber();

	ClearChannels();
	AddChannel( mInputChannel, "Dshot", true );

	return true;
}

void DshotAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mDshotRateInterface->SetNumber(mDshotRate);
}

void DshotAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	text_archive >> mDshotRate;

	ClearChannels();
	AddChannel( mInputChannel, "Dshot", true );

	UpdateInterfacesFromSettings();
}

const char* DshotAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mDshotRate;

	return SetReturnString( text_archive.GetString() );
}
