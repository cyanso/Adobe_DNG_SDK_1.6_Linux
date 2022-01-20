/*****************************************************************************/
// Copyright 2008-2019 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

#include "dng_gain_map.h"

#include "dng_exceptions.h"
#include "dng_globals.h"
#include "dng_host.h"
#include "dng_negative.h"
#include "dng_pixel_buffer.h"
#include "dng_sdk_limits.h"
#include "dng_stream.h"
#include "dng_tag_values.h"

/*****************************************************************************/

class dng_gain_map_interpolator
	{
	
	private:
	
		const dng_gain_map &fMap;
		
		dng_point_real64 fScale;
		dng_point_real64 fOffset;
		
		int32 fColumn;
		int32 fPlane;
		
		uint32 fRowIndex1;
		uint32 fRowIndex2;
		real32 fRowFract;
		
		int32 fResetColumn;
		
		real32 fValueBase;
		real32 fValueStep;
		real32 fValueIndex;
			
	public:
	
		dng_gain_map_interpolator (const dng_gain_map &map,
								   const dng_rect &mapBounds,
								   int32 row,
								   int32 column,
								   uint32 plane);

		real32 Interpolate () const
			{
			
			return fValueBase + fValueStep * fValueIndex;
			
			}
			
		void Increment ()
			{
			
			if (++fColumn >= fResetColumn)
				{
				
				ResetColumn ();
				
				}
				
			else
				{
				
				fValueIndex += 1.0f;
				
				}
						
			}
	
	private:
			
		real32 InterpolateEntry (uint32 colIndex);
			
		void ResetColumn ();
			
	};
			
/*****************************************************************************/

dng_gain_map_interpolator::dng_gain_map_interpolator (const dng_gain_map &map,
													  const dng_rect &mapBounds,
													  int32 row,
													  int32 column,
													  uint32 plane)

	:	fMap (map)
	
	,	fScale (1.0 / mapBounds.H (),
				1.0 / mapBounds.W ())
	
	,	fOffset (0.5 - mapBounds.t,
				 0.5 - mapBounds.l)
	
	,	fColumn (column)
	,	fPlane	(plane)
	
	,	fRowIndex1 (0)
	,	fRowIndex2 (0)
	,	fRowFract  (0.0f)
	
	,	fResetColumn (0)
	
	,	fValueBase	(0.0f)
	,	fValueStep	(0.0f)
	,	fValueIndex (0.0f)
	
	{
	
	real64 rowIndexF = (fScale.v * (row + fOffset.v) -
						fMap.Origin ().v) / fMap.Spacing ().v;
	
	if (rowIndexF <= 0.0)
		{
		
		fRowIndex1 = 0;
		fRowIndex2 = 0;
		
		fRowFract = 0.0f;

		}
		
	else
		{
		
		if (fMap.Points ().v < 1)
			{
			ThrowProgramError ("Empty gain map");
			}

		uint32 lastRow = static_cast<uint32> (fMap.Points ().v - 1);
		
		if (rowIndexF >= static_cast<real64> (lastRow))
			{
			
			fRowIndex1 = lastRow;
			fRowIndex2 = fRowIndex1;
			
			fRowFract = 0.0f;
			
			}
			
		else
			{
			
			// If we got here, we know that rowIndexF can safely be converted to
			// a uint32 and that static_cast<uint32> (rowIndexF) < lastRow. This
			// implies fRowIndex2 <= lastRow below.

			fRowIndex1 = static_cast<uint32> (rowIndexF);
			fRowIndex2 = fRowIndex1 + 1;
			
			fRowFract = (real32) (rowIndexF - (real64) fRowIndex1);
			
			}
			
		}
	
	ResetColumn ();
		
	}

/*****************************************************************************/

real32 dng_gain_map_interpolator::InterpolateEntry (uint32 colIndex)
	{
	
	return fMap.Entry (fRowIndex1, colIndex, fPlane) * (1.0f - fRowFract) +
		   fMap.Entry (fRowIndex2, colIndex, fPlane) * (	   fRowFract);
	
	}

/*****************************************************************************/

void dng_gain_map_interpolator::ResetColumn ()
	{
	
	real64 colIndexF = ((fScale.h * (fColumn + fOffset.h)) - 
						fMap.Origin ().h) / fMap.Spacing ().h;
	
	if (colIndexF <= 0.0)
		{
		
		fValueBase = InterpolateEntry (0);
		
		fValueStep = 0.0f;
		
		fResetColumn = (int32) ceil (fMap.Origin ().h / fScale.h - fOffset.h);

		}
		
	else
		{
	
		if (fMap.Points ().h < 1)
			{
			ThrowProgramError ("Empty gain map");
			}

		uint32 lastCol = static_cast<uint32> (fMap.Points ().h - 1);
		
		if (colIndexF >= static_cast<real64> (lastCol))
			{
			
			fValueBase = InterpolateEntry (lastCol);
			
			fValueStep = 0.0f;
			
			fResetColumn = 0x7FFFFFFF;
			
			}
		
		else
			{
			
			// If we got here, we know that colIndexF can safely be converted
			// to a uint32 and that static_cast<uint32> (colIndexF) < lastCol.
			// This implies colIndex + 1 <= lastCol, i.e. the argument to
			// InterpolateEntry() below is valid.

			uint32 colIndex = static_cast<uint32> (colIndexF);

			real64 base	 = InterpolateEntry (colIndex);
			real64 delta = InterpolateEntry (colIndex + 1) - base;
			
			fValueBase = (real32) (base + delta * (colIndexF - (real64) colIndex));
			
			fValueStep = (real32) ((delta * fScale.h) / fMap.Spacing ().h);
			
			fResetColumn = (int32) ceil (((colIndex + 1) * fMap.Spacing ().h +
										  fMap.Origin ().h) / fScale.h - fOffset.h);
			
			}
			
		}
	
	fValueIndex = 0.0f;
	
	}

/*****************************************************************************/

DNG_ATTRIB_NO_SANITIZE("unsigned-integer-overflow")
dng_gain_map::dng_gain_map (dng_memory_allocator &allocator,
							const dng_point &points,
							const dng_point_real64 &spacing,
							const dng_point_real64 &origin,
							uint32 planes)
					
	:	fPoints	 (points)
	,	fSpacing (spacing)
	,	fOrigin	 (origin)
	,	fPlanes	 (planes)
	
	,	fRowStep (planes * points.h)
	
	,	fBuffer ()
	
	{
	
	fBuffer.Reset (allocator.Allocate (ComputeBufferSize (ttFloat, 
														  fPoints, 
														  fPlanes, 
														  padSIMDBytes)));
	
	}
					  
/*****************************************************************************/

real32 dng_gain_map::Interpolate (int32 row,
								  int32 col,
								  uint32 plane,
								  const dng_rect &bounds) const
	{
	
	dng_gain_map_interpolator interp (*this,
									  bounds,
									  row,
									  col,
									  plane);
									  
	return interp.Interpolate ();
	
	}
					  
/*****************************************************************************/

uint32 dng_gain_map::PutStreamSize () const
	{
	
	return 44 + fPoints.v * fPoints.h * fPlanes * 4;
	
	}
					  
/*****************************************************************************/

void dng_gain_map::PutStream (dng_stream &stream) const
	{
	
	stream.Put_uint32 (fPoints.v);
	stream.Put_uint32 (fPoints.h);
	
	stream.Put_real64 (fSpacing.v);
	stream.Put_real64 (fSpacing.h);
	
	stream.Put_real64 (fOrigin.v);
	stream.Put_real64 (fOrigin.h);
	
	stream.Put_uint32 (fPlanes);
	
	for (int32 rowIndex = 0; rowIndex < fPoints.v; rowIndex++)
		{
		
		for (int32 colIndex = 0; colIndex < fPoints.h; colIndex++)
			{
			
			for (uint32 plane = 0; plane < fPlanes; plane++)
				{
				
				stream.Put_real32 (Entry (rowIndex,
										  colIndex,
										  plane));
										  
				}
				
			}
			
		}
	
	}

/*****************************************************************************/

dng_gain_map * dng_gain_map::GetStream (dng_host &host,
										dng_stream &stream)
	{
	
	dng_point mapPoints;
	
	mapPoints.v = stream.Get_uint32 ();
	mapPoints.h = stream.Get_uint32 ();
	
	dng_point_real64 mapSpacing;
	
	mapSpacing.v = stream.Get_real64 ();
	mapSpacing.h = stream.Get_real64 ();
	
	dng_point_real64 mapOrigin;
	
	mapOrigin.v = stream.Get_real64 ();
	mapOrigin.h = stream.Get_real64 ();
	
	uint32 mapPlanes = stream.Get_uint32 ();
	
	#if qDNGValidate
	
	if (gVerbose)
		{
		
		printf ("Points: v=%d, h=%d\n", 
				(int) mapPoints.v,
				(int) mapPoints.h);
		
		printf ("Spacing: v=%.6f, h=%.6f\n", 
				mapSpacing.v,
				mapSpacing.h);
		
		printf ("Origin: v=%.6f, h=%.6f\n", 
				mapOrigin.v,
				mapOrigin.h);
		
		printf ("Planes: %u\n", 
				(unsigned) mapPlanes);
		
		}
		
	#endif
	
	if (mapPoints.v == 1)
		{
		mapSpacing.v = 1.0;
		mapOrigin.v	 = 0.0;
		}
	
	if (mapPoints.h == 1)
		{
		mapSpacing.h = 1.0;
		mapOrigin.h	 = 0.0;
		}
		
	if (mapPoints.v < 1 ||
		mapPoints.h < 1 ||
		mapSpacing.v <= 0.0 ||
		mapSpacing.h <= 0.0 ||
		mapPlanes < 1)
		{
		ThrowBadFormat ();
		}
	
	AutoPtr<dng_gain_map> map (new dng_gain_map (host.Allocator (),
												 mapPoints,
												 mapSpacing,
												 mapOrigin,
												 mapPlanes));
												 
	#if qDNGValidate
	
	uint32 linesPrinted = 0;
	uint32 linesSkipped = 0;
	
	#endif
	
	for (int32 rowIndex = 0; rowIndex < mapPoints.v; rowIndex++)
		{
		
		for (int32 colIndex = 0; colIndex < mapPoints.h; colIndex++)
			{
			
			for (uint32 plane = 0; plane < mapPlanes; plane++)
				{
				
				real32 x = stream.Get_real32 ();
				
				map->Entry (rowIndex, colIndex, plane) = x;
				
				#if qDNGValidate
				
				if (gVerbose)
					{
					
					if (linesPrinted < gDumpLineLimit)
						{
						
						printf ("\tMap [%3u] [%3u] [%u] = %.4f\n",
								(unsigned) rowIndex,
								(unsigned) colIndex,
								(unsigned) plane,
								x);
						
						linesPrinted++;
						
						}
						
					else
						linesSkipped++;
						
					}
					
				#endif

				}
				
			}
			
		}
												 
	#if qDNGValidate
	
	if (linesSkipped)
		{
		
		printf ("\t ... %u map entries skipped\n", (unsigned) linesSkipped);
		
		}
	
	#endif
				
	return map.Release ();
	
	}

/*****************************************************************************/

#if qDNGProfileGainTableMap

/*****************************************************************************/

DNG_ATTRIB_NO_SANITIZE("unsigned-integer-overflow")
dng_gain_table_map::dng_gain_table_map (dng_memory_allocator &allocator,
										const dng_point &points,
										const dng_point_real64 &spacing,
										const dng_point_real64 &origin,
										uint32 numTablePoints,
										const real32 weights [5])

	:	fPoints	 (points)
	,	fSpacing (spacing)
	,	fOrigin	 (origin)
	,	fNumTablePoints (numTablePoints)
	
	,	fRowStep (fNumTablePoints * points.h)
	,	fColStep (fNumTablePoints)

	,	fNumSamples (SafeUint32Mult (points.h,
									 points.v,
									 numTablePoints))

	{

	fSampleBytes = SafeUint32Mult (fNumSamples,
								   (uint32) sizeof (real32));

	if (fNumSamples >= kMaxProfileGainTableMapPoints)
		{
		
		ThrowBadFormat ("too many points in gain table map");
		
		}

	memcpy (fMapInputWeights,
			weights,
			sizeof (fMapInputWeights));
	
	fBuffer.Reset (allocator.Allocate (ComputeBufferSize (ttFloat, 
														  fPoints, 
														  numTablePoints,
														  padSIMDBytes)));
	
	}

/*****************************************************************************/

void * dng_gain_table_map::RawTablePtr () const
	{

	DNG_REQUIRE (fBuffer.Get (), "fBuffer");

	// The table currently starts at the beginning of the block's Buffer.

	return fBuffer->Buffer ();

	}

/*****************************************************************************/

uint32 dng_gain_table_map::RawTableNumBytes () const
	{

	// This is a real32 table.

	return fNumSamples * sizeof (real32);

	}

/*****************************************************************************/

uint32 dng_gain_table_map::PutStreamSize () const
	{

	// 4
	// 4
	// 8
	// 8
	// 8
	// 8
	// 4
	// 5*4

	return ((2 * 4) +						 // MapPointsV, MapPointsH
			(2 * 8) +						 // MapSpacingV, MapSpacingH
			(2 * 8) +						 // MapOriginV, MapOriginH
			(	 4) +						 // MapPoints
			(5 * 4) +						 // MapInputWeights
			fPoints.v * fPoints.h * fNumTablePoints * 4); // data
	
	}

/*****************************************************************************/

void dng_gain_table_map::AddDigest (dng_md5_printer &printer) const
	{

	printer.Process ("ProfileGainTableMap", 19);

	EnsureFingerprint ();

	printer.Process (fFingerprint.data, dng_fingerprint::kDNGFingerprintSize);

	}

/*****************************************************************************/

void dng_gain_table_map::EnsureFingerprint () const
	{

	if (fFingerprint.IsNull ())
		{

		dng_md5_printer printer;

		dng_md5_printer_stream stream;

		PutStream (stream);

		fFingerprint = printer.Result ();

		}

	}

/*****************************************************************************/

dng_fingerprint dng_gain_table_map::GetFingerprint () const
	{

	EnsureFingerprint ();

	return fFingerprint;

	}

/*****************************************************************************/

void dng_gain_table_map::PutStream (dng_stream &stream) const
	{
	
	stream.Put_uint32 (fPoints.v);
	stream.Put_uint32 (fPoints.h);
	
	stream.Put_real64 (fSpacing.v);
	stream.Put_real64 (fSpacing.h);
	
	stream.Put_real64 (fOrigin.v);
	stream.Put_real64 (fOrigin.h);
	
	stream.Put_uint32 (fNumTablePoints);

	for (uint32 i = 0; i < 5; i++)
		{
		stream.Put_real32 (fMapInputWeights [i]);
		}
	
	for (int32 rowIndex = 0; rowIndex < fPoints.v; rowIndex++)
		{
		
		for (int32 colIndex = 0; colIndex < fPoints.h; colIndex++)
			{
			
			for (uint32 p = 0; p < fNumTablePoints; p++)
				{
				
				stream.Put_real32 (Entry (rowIndex,
										  colIndex,
										  p));
										  
				}
				
			}
			
		}
	
	}

/*****************************************************************************/

dng_gain_table_map * dng_gain_table_map::GetStream (dng_host &host,
													dng_stream &stream)
	{
	
	dng_point mapPoints;
	
	mapPoints.v = stream.Get_uint32 ();
	mapPoints.h = stream.Get_uint32 ();
	
	dng_point_real64 mapSpacing;
	
	mapSpacing.v = stream.Get_real64 ();
	mapSpacing.h = stream.Get_real64 ();
	
	dng_point_real64 mapOrigin;
	
	mapOrigin.v = stream.Get_real64 ();
	mapOrigin.h = stream.Get_real64 ();
	
	uint32 numTablePoints = stream.Get_uint32 ();

	real32 weights [5];

	for (uint32 i = 0; i < 5; i++)
		{
		weights [i] = stream.Get_real32 ();
		}
	
	#if qDNGValidate
	
	if (gVerbose)
		{

		printf ("GainTableMap:\n");
		
		printf ("  Points: v=%d, h=%d\n", 
				(int) mapPoints.v,
				(int) mapPoints.h);
		
		printf ("  Spacing: v=%.6f, h=%.6f\n", 
				mapSpacing.v,
				mapSpacing.h);
		
		printf ("  Origin: v=%.6f, h=%.6f\n", 
				mapOrigin.v,
				mapOrigin.h);
		
		printf ("  NumTablePoints: %u\n", 
				(unsigned) numTablePoints);

		printf ("  Weights: %.4f, %.4f, %.4f, %.4f, %.4f\n",
				(float) weights [0],
				(float) weights [1],
				(float) weights [2],
				(float) weights [3],
				(float) weights [4]);
				
		}
		
	#endif
	
	if (mapPoints.v == 1)
		{
		mapSpacing.v = 1.0;
		mapOrigin.v	 = 0.0;
		}
	
	if (mapPoints.h == 1)
		{
		mapSpacing.h = 1.0;
		mapOrigin.h	 = 0.0;
		}
		
	if (mapPoints.v < 1 ||
		mapPoints.h < 1 ||
		mapSpacing.v <= 0.0 ||
		mapSpacing.h <= 0.0 ||
		numTablePoints < 1)
		{
		ThrowBadFormat ();
		}

	// Check the weights.

	AutoPtr<dng_gain_table_map> map
		(new dng_gain_table_map (host.Allocator (),
								 mapPoints,
								 mapSpacing,
								 mapOrigin,
								 numTablePoints,
								 weights));
												 
	#if qDNGValidate
	
	uint32 linesPrinted = 0;
	uint32 linesSkipped = 0;
	
	#endif
	
	for (int32 rowIndex = 0; rowIndex < mapPoints.v; rowIndex++)
		{
		
		for (int32 colIndex = 0; colIndex < mapPoints.h; colIndex++)
			{
			
			for (uint32 p = 0; p < numTablePoints; p++)
				{
				
				real32 x = stream.Get_real32 ();
				
				map->Entry (rowIndex, colIndex, p) = x;
				
				#if qDNGValidate
				
				if (gVerbose)
					{
					
					if (linesPrinted < gDumpLineLimit)
						{
						
						printf ("\tMap [%3u] [%3u] [%u] = %.4f\n",
								(unsigned) rowIndex,
								(unsigned) colIndex,
								(unsigned) p,
								x);
						
						linesPrinted++;
						
						}
						
					else
						linesSkipped++;
						
					}
					
				#endif

				// Check range and bad values.

				if (x < kProfileGainTableMap_MinGainValue ||
					x > kProfileGainTableMap_MaxGainValue)
					{
					ThrowBadFormat ("ProfileGainTableMap entry value out of range");
					}

				if (x != x)
					{
					ThrowBadFormat ("Invalid ProfileGainTableMap entry value");
					}

				}
				
			}
			
		}
												 
	#if qDNGValidate
	
	if (linesSkipped)
		{
		
		printf ("\t ... %u map entries skipped\n", (unsigned) linesSkipped);
		
		}
	
	#endif
				
	return map.Release ();
	
	}

/*****************************************************************************/

#endif	// qDNGProfileGainTableMap

/*****************************************************************************/

dng_opcode_GainMap::dng_opcode_GainMap (const dng_area_spec &areaSpec,
										AutoPtr<dng_gain_map> &gainMap)
	
	:	dng_inplace_opcode (dngOpcode_GainMap,
							dngVersion_1_3_0_0,
							kFlag_None)
							
	,	fAreaSpec (areaSpec)
					
	,	fGainMap ()
	
	{
	
	fGainMap.Reset (gainMap.Release ());
	
	}
		
/*****************************************************************************/

dng_opcode_GainMap::dng_opcode_GainMap (dng_host &host,
										dng_stream &stream)
												
	:	dng_inplace_opcode (dngOpcode_GainMap,
							stream,
							"GainMap")
							
	,	fAreaSpec ()
							
	,	fGainMap ()
							
	{
	
	uint32 byteCount = stream.Get_uint32 ();
	
	uint64 startPosition = stream.Position ();
	
	fAreaSpec.GetData (stream);
	
	fGainMap.Reset (dng_gain_map::GetStream (host, stream));
	
	if (stream.Position () != startPosition + byteCount)
		{
		ThrowBadFormat ();
		}
		
	}
		
/*****************************************************************************/

void dng_opcode_GainMap::PutData (dng_stream &stream) const
	{
	
	stream.Put_uint32 (dng_area_spec::kDataSize +
					   fGainMap->PutStreamSize ());
					   
	fAreaSpec.PutData (stream);
	
	fGainMap->PutStream (stream);
	
	}
		
/*****************************************************************************/

void dng_opcode_GainMap::ProcessArea (dng_negative &negative,
									  uint32 /* threadIndex */,
									  dng_pixel_buffer &buffer,
									  const dng_rect &dstArea,
									  const dng_rect &imageBounds)
	{

	dng_rect overlap = fAreaSpec.ScaledOverlap (dstArea);
	
	if (overlap.NotEmpty ())
		{
  
		uint16 blackLevel = (Stage () >= 2) ? negative.Stage3BlackLevel () : 0;
		
		real32 blackScale1	= 1.0f;
		real32 blackScale2	= 1.0f;
		real32 blackOffset1 = 0.0f;
		real32 blackOffset2 = 0.0f;

		if (blackLevel != 0)
			{
			
			blackOffset2 = ((real32) blackLevel) / 65535.0f;
			blackScale2	 = 1.0f - blackOffset2;
			blackScale1	 = 1.0f / blackScale2;
			blackOffset1 = 1.0f - blackScale1;
			
			}
		
		uint32 cols = overlap.W ();
		
		uint32 colPitch = fAreaSpec.ColPitch ();
		
		colPitch = Min_uint32 (colPitch, cols);
		
		for (uint32 plane = fAreaSpec.Plane ();
			 plane < fAreaSpec.Plane () + fAreaSpec.Planes () &&
			 plane < buffer.Planes ();
			 plane++)
			{
			
			uint32 mapPlane = Min_uint32 (plane, fGainMap->Planes () - 1);
			
			for (int32 row = overlap.t; row < overlap.b; row += fAreaSpec.RowPitch ())
				{
				
				real32 *dPtr = buffer.DirtyPixel_real32 (row, overlap.l, plane);
				
				dng_gain_map_interpolator interp (*fGainMap,
												  imageBounds,
												  row,
												  overlap.l,
												  mapPlane);
			  
				if (blackLevel != 0)
					{
					
					for (uint32 col = 0; col < cols; col += colPitch)
						{

						dPtr [col] = dPtr [col] * blackScale1 + blackOffset1;
								
						}
						
					}
					
				for (uint32 col = 0; col < cols; col += colPitch)
					{
					
					real32 gain = interp.Interpolate ();
					
					dPtr [col] = Min_real32 (dPtr [col] * gain, 1.0f);
					
					for (uint32 j = 0; j < colPitch; j++)
						{
						interp.Increment ();
						}
					
					}
				
				if (blackLevel != 0)
					{
					
					for (uint32 col = 0; col < cols; col += colPitch)
						{

						dPtr [col] = dPtr [col] * blackScale2 + blackOffset2;
							
						}
						
					}
					
				}
			
			}
		
		}

	}
	
/*****************************************************************************/
