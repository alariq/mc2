#pragma once

//===========================================================================//
// File:	 ACM.cpp														 //
// Contents: Wrappers for the Audio Compression Manager						 //																		 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

MMRESULT wACMStreamPrepareHeader( HACMSTREAM hACMStream, ACMSTREAMHEADER *ACMHeader, DWORD fdwPrepare );
MMRESULT wACMStreamConvert( HACMSTREAM hACMStream, ACMSTREAMHEADER *ACMHeader, DWORD flags );
MMRESULT wACMStreamUnprepareHeader( HACMSTREAM hACMStream, ACMSTREAMHEADER *ACMHeader, DWORD fdwPrepare );
MMRESULT wACMStreamClose( HACMSTREAM hACMStream, DWORD flags );
MMRESULT wACMStreamOpen( LPHACMSTREAM phas, HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, LPWAVEFILTER pwfltr, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen );
MMRESULT wACMStreamSize( HACMSTREAM has, DWORD cbInput, LPDWORD pdwOutputBytes, DWORD fdwSize );
MMRESULT wACMFormatSuggest( HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, DWORD cbwfxDst, DWORD fdwSuggest );
