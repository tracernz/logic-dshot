#include "DshotAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "DshotAnalyzer.h"
#include "DshotAnalyzerSettings.h"
#include <iostream>
#include <fstream>

DshotAnalyzerResults::DshotAnalyzerResults( DshotAnalyzer* analyzer, DshotAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

DshotAnalyzerResults::~DshotAnalyzerResults()
{
}

void DshotAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);

	char number_str[128];
	AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 11, number_str, 128);
	const char *telem_str = frame.mFlags & 1 ? " (telem req.)" : "";
	const char *error_str = frame.mFlags & DISPLAY_AS_ERROR_FLAG ? "!" : "";

	// widest display
	AddResultString(error_str, number_str, telem_str);
	// middle
	AddResultString(error_str, number_str);
	// shortest
	if (frame.mFlags & DISPLAY_AS_ERROR_FLAG)
		AddResultString(error_str);
	else
		AddResultString(number_str);
}

void DshotAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 11, number_str, 128);

		file_stream << time_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void DshotAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
	Frame frame = GetFrame( frame_index );
	ClearResultStrings();

	char number_str[128];
	AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 11, number_str, 128);
	AddResultString( number_str );
}

void DshotAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void DshotAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}