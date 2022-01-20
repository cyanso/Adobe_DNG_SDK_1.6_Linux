/*****************************************************************************/
// Copyright 2006-2019 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:	Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

#include "dng_xy_coord.h"

#include "dng_1d_function.h"
#include "dng_assertions.h"
#include "dng_globals.h"
#include "dng_matrix.h"
#include "dng_point.h"
#include "dng_stream.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_temperature.h"
#include "dng_utils.h"

/******************************************************************************/

dng_xy_coord XYZtoXY (const dng_vector_3 &coord)
	{
	
	real64 X = coord [0];
	real64 Y = coord [1];
	real64 Z = coord [2];
	
	real64 total = X + Y + Z;
	
	if (total > 0.0)
		{
		
		return dng_xy_coord (X / total,
							 Y / total);
						
		}
		
	return D50_xy_coord ();
		
	}

/*****************************************************************************/

dng_vector_3 XYtoXYZ (const dng_xy_coord &coord)
	{
	
	dng_xy_coord temp = coord;
	
	// Restrict xy coord to someplace inside the range of real xy coordinates.
	// This prevents math from doing strange things when users specify
	// extreme temperature/tint coordinates.
	
	temp.x = Pin_real64 (0.000001, temp.x, 0.999999);
	temp.y = Pin_real64 (0.000001, temp.y, 0.999999);
	
	if (temp.x + temp.y > 0.999999)
		{
		real64 scale = 0.999999 / (temp.x + temp.y);
		temp.x *= scale;
		temp.y *= scale;
		}
	
	return dng_vector_3 (temp.x / temp.y,
						 1.0,
						 (1.0 - temp.x - temp.y) / temp.y);
	
	}

/*****************************************************************************/

dng_xy_coord PCStoXY ()
	{
	
	return D50_xy_coord ();
	
	}

/*****************************************************************************/

dng_vector_3 PCStoXYZ ()
	{
	
	return XYtoXYZ (PCStoXY ());
	
	}

/*****************************************************************************/

dng_illuminant_data::dng_illuminant_data (const uint32 light,
										  const dng_illuminant_data *otherDataPtr)
	{
	
	switch (light)
		{
		
		case lsStandardLightA:
		case lsTungsten:
			{
			SetWhiteXY (StdA_xy_coord ());
			break;
			}
			
		case lsISOStudioTungsten:
			{
			dng_temperature temp (3200.0, 0.0);
			SetWhiteXY (temp.Get_xy_coord ());
			break;
			}
			
		case lsStandardLightB:
			{
			SetWhiteXY (dng_xy_coord (0.348483, 0.351747)); // 4871.4 K
			break;
			}

		case lsStandardLightC:
			{
			SetWhiteXY (dng_xy_coord (0.310061, 0.316150)); // 6774.3 K
			break;
			}
			
		case lsD50:
			{
			SetWhiteXY (D50_xy_coord ());
			break;
			}
			
		case lsD55:
		case lsDaylight:
		case lsFineWeather:
		case lsFlash:
			{
			SetWhiteXY (D55_xy_coord ());
			break;
			}
			
		case lsD65:
		case lsCloudyWeather:
			{
			SetWhiteXY (D65_xy_coord ());
			break;
			}
			
		case lsD75:
		case lsShade:
			{
			SetWhiteXY (D75_xy_coord ());
			break;
			}
			
		case lsDaylightFluorescent:
			{
			// Use F1: ~6430 K.
			SetWhiteXY (dng_xy_coord (0.31310, 0.33727));
			break;
			}
			
		case lsDayWhiteFluorescent:
			{
			// Use F8: ~5000 K.
			SetWhiteXY (dng_xy_coord (0.34588, 0.35875));
			break;
			}
			
		case lsCoolWhiteFluorescent:
			{
			// Use F9: ~4150 K
			SetWhiteXY (dng_xy_coord (0.37417, 0.37281));
			break;
			}

		case lsFluorescent:
			{
			// Use F2: ~4230 K
			SetWhiteXY (dng_xy_coord (0.37208, 0.37529));
			break;
			}
			
		case lsWhiteFluorescent:
			{
			// Use F3: ~3450 K
			SetWhiteXY (dng_xy_coord (0.40910, 0.39430));
			break;
			}
			
		case lsWarmWhiteFluorescent:
			{
			// Use F4: ~2940 K
			SetWhiteXY (dng_xy_coord (0.44018, 0.40329));
			break;
			}

		case lsOther:
			{

			DNG_REQUIRE (otherDataPtr != nullptr,
						 "Missing otherDataPtr for lsOther");

			*this = *otherDataPtr;

			break;

			}
			
		default:
			{
			SetWhiteXY (D50_xy_coord ());
			break;
			}

		}
	
	}

/*****************************************************************************/

void dng_illuminant_data::Clear ()
	{
	
	*this = dng_illuminant_data ();	
	
	}

/*****************************************************************************/

bool dng_illuminant_data::IsValid () const
	{
	
	if (fType == kWhiteXY)
		{

		return (fWhiteX.IsValid () &&
				fWhiteY.IsValid () &&
				WhiteXY ().IsValid ());
		
		}

	else if (fType == kSpectrum)
		{
		
		if (fMinLambda.NotValid ())
			{
			return false;
			}

		if (fLambdaSpacing.NotValid ())
			{
			return false;
			}

		if (fSpectrum.size () < (size_t) kMinSpectrumSamples ||
			fSpectrum.size () > (size_t) kMaxSpectrumSamples)
			{
			return false;
			}

		return WhiteXY ().IsValid ();
		
		}

	// Not valid.

	return false;
	
	}

/*****************************************************************************/

void dng_illuminant_data::SetWhiteXY (const dng_xy_coord &white)
	{

	dng_urational x;
	dng_urational y;

	x.Set_real64 (white.x);
	y.Set_real64 (white.y);

	SetWhiteXY (x, y);

	}

/*****************************************************************************/

void dng_illuminant_data::SetWhiteXY (const dng_urational &x,
									  const dng_urational &y)
	{

	if (x.NotValid () || y.NotValid ())
		{
		
		ThrowBadFormat ("Invalid x or y in dng_illuminant_data::SetWhiteXY");
		
		}

	const real64 kMinValue = 0.000001;
	const real64 kMaxValue = 0.999999;

	real64 x64 = x.As_real64 ();
	real64 y64 = y.As_real64 ();

	if (x64 < kMinValue || x64 > kMaxValue ||
		y64 < kMinValue || y64 > kMaxValue)
		{
		
		ThrowBadFormat ("Out-of-range x or y in dng_illuminant_data::SetWhiteXY");
		
		}

	// Set type.

	fType = kWhiteXY;

	// Set XY data.

	fWhiteX = x;
	fWhiteY = y;

	// For the derived white, store a cleaned-up version of the xy white by
	// round-tripping it through the xy/XYZ conversion process.

	fDerivedWhite = XYZtoXY (XYtoXYZ (dng_xy_coord (x64, y64)));

	// Clear spectrum data.

	fMinLambda.Clear ();

	fLambdaSpacing.Clear ();

	fSpectrum.clear ();

	}

/*****************************************************************************/

void dng_illuminant_data::SetSpectrum (const dng_urational &minLambda,
									   const dng_urational &lambdaSpacing,
									   const std::vector<dng_urational> &data)
	{

	// Verify spectrum data.

	DNG_REQUIRE (minLambda.As_real64 () > 0.0,
				 "Invalid minLambda");
				 
	DNG_REQUIRE (lambdaSpacing.As_real64 () > 0.0,
				 "Invalid lambdaSpacing");

	DNG_REQUIRE (data.size () >= (size_t) kMinSpectrumSamples,
				 "Too few spectral samples");

	DNG_REQUIRE (data.size () <= (size_t) kMaxSpectrumSamples,
				 "Too many spectral samples");

	#if qDNGValidate
				
	real64 maxLambda =
		minLambda.As_real64 () + (lambdaSpacing.As_real64 () * (data.size () - 1));

	if (minLambda.As_real64 () > 400.0 ||
		maxLambda			   < 700.0)
		{
		
		ReportWarning ("spectrum data doesn't cover at least [400,700] nm");
				
		}

	#endif
		
	// Set type.

	fType = kSpectrum;
	
	// Clear XY data.

	fWhiteX.Clear ();
	fWhiteY.Clear ();

	// Set spectrum data.

	fMinLambda = minLambda;

	fLambdaSpacing = lambdaSpacing;

	fSpectrum = data;

	// Calculate white from spectrum.

	CalculateSpectrumXY ();

	}

/*****************************************************************************/

void dng_illuminant_data::Get (dng_stream &stream,
							   const uint32 tagCount,
							   const char *tagName)
	{
	
	(void) tagName;
	
	uint16 type = stream.Get_uint16 ();

	if (type == 0)
		{

		uint32 expectedTagBytes = 2 + 8 + 8;

		if (tagCount < expectedTagBytes)
			{

			ThrowBadFormat ("tag count too small for illuminant xy data");
			
			}
				
		dng_urational x = stream.TagValue_urational (ttRational);
		dng_urational y = stream.TagValue_urational (ttRational);

		SetWhiteXY (x, y);

		#if qDNGValidate

		if (gVerbose)
			{

			dng_temperature tempTint (WhiteXY ());
			
			printf ("%s: x-y type, x=%u/%u (%.6lf), y=%u/%u (%.6lf), temp=%.1lf, tint=%.1lf\n",
					tagName,
					x.n,
					x.d,
					x.As_real64 (),
					y.n,
					y.d,
					y.As_real64 (),
					tempTint.Temperature (),
					tempTint.Tint ());
			
			}

		#endif	// qDNGValidate
				
		}

	else if (type == 1)
		{

		uint32 numSamples = stream.Get_uint32 ();

		if (numSamples < kMinSpectrumSamples ||
			numSamples > kMaxSpectrumSamples)
			{

			ThrowBadFormat ("invalid sample count for illuminant spectrum data");
			
			}

		uint32 expectedTagBytes = (2 +			 // type
								   4 +			 // num samples
								   8 +			 // min lambda
								   8 +			 // spacing
								   (numSamples * 8)); // samples

		if (tagCount < expectedTagBytes)
			{

			ThrowBadFormat ("tag count too small for illuminant spectrum data");

			}

		dng_urational minLambda = stream.TagValue_urational (ttRational);
				
		dng_urational spacing = stream.TagValue_urational (ttRational);

		std::vector<dng_urational> samples (numSamples);

		for (uint32 i = 0; i < numSamples; i++)
			{
			
			samples [i] = stream.TagValue_urational (ttRational);
			
			}

		SetSpectrum (minLambda,
					 spacing,
					 samples);
		
		#if qDNGValidate

		if (gVerbose)
			{

			const dng_xy_coord &xy = WhiteXY ();

			dng_temperature tempTint (xy);
			
			printf ("%s: spectrum type, numSamples=%u, minLambda=%u/%u (%.1lf), "
					"spacing=%u/%u (%.1lf), x=%.6lf, y=%.6lf, temp=%.1lf, tint=%.1lf\n",
					tagName,
					numSamples,
					minLambda.n,
					minLambda.d,
					minLambda.As_real64 (),
					spacing.n,
					spacing.d,
					spacing.As_real64 (),
					xy.x,
					xy.y,
					tempTint.Temperature (),
					tempTint.Tint ());

			real64 minLambda64 = minLambda.As_real64 ();
			real64 spacing64 = spacing.As_real64 ();

			for (uint32 i = 0; i < numSamples; i++)
				{

				real64 lambda = minLambda64 + (i * spacing64);

				const dng_urational &value = samples [i];
				
				printf ("%4u: %.8lf --> %8u/%8u (%.8lf)\n",
						i,
						lambda,
						value.n,
						value.d,
						value.As_real64 ());
				
				}
			
			}

		#endif	// qDNGValidate
				
		}
			
	else
		{

		ThrowBadFormat ("Unrecognized illuminant data type");
		
		}
	
	}

/*****************************************************************************/

void dng_illuminant_data::Put (dng_stream &stream) const
	{
	
	if (fType == kWhiteXY)
		{
		
		stream.Put_uint16 (0);

		stream.Put_uint32 (fWhiteX.n);
		stream.Put_uint32 (fWhiteX.d);
		
		stream.Put_uint32 (fWhiteY.n);
		stream.Put_uint32 (fWhiteY.d);
		
		}

	else if (fType == kSpectrum)
		{
		
		stream.Put_uint16 (1);

		stream.Put_uint32 ((uint32) fSpectrum.size ());

		stream.Put_uint32 (fMinLambda.n);
		stream.Put_uint32 (fMinLambda.d);

		stream.Put_uint32 (fLambdaSpacing.n);
		stream.Put_uint32 (fLambdaSpacing.d);

		for (const auto &sample : fSpectrum)
			{
			
			stream.Put_uint32 (sample.n);
			stream.Put_uint32 (sample.d);

			}
		
		}

	else
		{
		
		ThrowProgramError ("Invalid fType in dng_illuminant_data::Put");

		}
	
	}

/*****************************************************************************/

uint32 dng_illuminant_data::TagCount () const
	{
	
	if (fType == kWhiteXY)
		{
		
		return (2 +							 // type
				8 +							 // x
				8);							 // y
		
		}

	else if (fType == kSpectrum)
		{
		
		return (2 +							 // type
				4 +							 // num samples
				8 +							 // min lambda
				8 +							 // spacing
				(8 * (uint32) fSpectrum.size ())); // sample data
		
		}

	ThrowProgramError ("Invalid fType in TagCount");

	return 0;
	
	}

/*****************************************************************************/

// 360 to 830 nm in 1 nm steps.

static const real64 kCIEStdObserver2Degree [] [3] =
	{
	{ 0.0001299		 , 0.000003917	   , 0.0006061		},
	{ 0.000145847	 , 0.000004393581  , 0.0006808792	},
	{ 0.0001638021	 , 0.000004929604  , 0.0007651456	},
	{ 0.0001840037	 , 0.000005532136  , 0.0008600124	},
	{ 0.0002066902	 , 0.000006208245  , 0.0009665928	},
	{ 0.0002321		 , 0.000006965	   , 0.001086		},
	{ 0.000260728	 , 0.000007813219  , 0.001220586	},
	{ 0.000293075	 , 0.000008767336  , 0.001372729	},
	{ 0.000329388	 , 0.000009839844  , 0.001543579	},
	{ 0.000369914	 , 0.00001104323   , 0.001734286	},
	{ 0.0004149		 , 0.00001239	   , 0.001946		},
	{ 0.0004641587	 , 0.00001388641   , 0.002177777	},
	{ 0.000518986	 , 0.00001555728   , 0.002435809	},
	{ 0.000581854	 , 0.00001744296   , 0.002731953	},
	{ 0.0006552347	 , 0.00001958375   , 0.003078064	},
	{ 0.0007416		 , 0.00002202	   , 0.003486		},
	{ 0.0008450296	 , 0.00002483965   , 0.003975227	},
	{ 0.0009645268	 , 0.00002804126   , 0.00454088		},
	{ 0.001094949	 , 0.00003153104   , 0.00515832		},
	{ 0.001231154	 , 0.00003521521   , 0.005802907	},
	{ 0.001368		 , 0.000039		   , 0.006450001	},
	{ 0.00150205	 , 0.0000428264	   , 0.007083216	},
	{ 0.001642328	 , 0.0000469146	   , 0.007745488	},
	{ 0.001802382	 , 0.0000515896	   , 0.008501152	},
	{ 0.001995757	 , 0.0000571764	   , 0.009414544	},
	{ 0.002236		 , 0.000064		   , 0.01054999		},
	{ 0.002535385	 , 0.00007234421   , 0.0119658		},
	{ 0.002892603	 , 0.00008221224   , 0.01365587		},
	{ 0.003300829	 , 0.00009350816   , 0.01558805		},
	{ 0.003753236	 , 0.0001061361	   , 0.01773015		},
	{ 0.004243		 , 0.00012		   , 0.02005001		},
	{ 0.004762389	 , 0.000134984	   , 0.02251136		},
	{ 0.005330048	 , 0.000151492	   , 0.02520288		},
	{ 0.005978712	 , 0.000170208	   , 0.02827972		},
	{ 0.006741117	 , 0.000191816	   , 0.03189704		},
	{ 0.00765		 , 0.000217		   , 0.03621		},
	{ 0.008751373	 , 0.0002469067	   , 0.04143771		},
	{ 0.01002888	 , 0.00028124	   , 0.04750372		},
	{ 0.0114217		 , 0.00031852	   , 0.05411988		},
	{ 0.01286901	 , 0.0003572667	   , 0.06099803		},
	{ 0.01431		 , 0.000396		   , 0.06785001		},
	{ 0.01570443	 , 0.0004337147	   , 0.07448632		},
	{ 0.01714744	 , 0.000473024	   , 0.08136156		},
	{ 0.01878122	 , 0.000517876	   , 0.08915364		},
	{ 0.02074801	 , 0.0005722187	   , 0.09854048		},
	{ 0.02319		 , 0.00064		   , 0.1102			},
	{ 0.02620736	 , 0.00072456	   , 0.1246133		},
	{ 0.02978248	 , 0.0008255	   , 0.1417017		},
	{ 0.03388092	 , 0.00094116	   , 0.1613035		},
	{ 0.03846824	 , 0.00106988	   , 0.1832568		},
	{ 0.04351		 , 0.00121		   , 0.2074			},
	{ 0.0489956		 , 0.001362091	   , 0.2336921		},
	{ 0.0550226		 , 0.001530752	   , 0.2626114		},
	{ 0.0617188		 , 0.001720368	   , 0.2947746		},
	{ 0.069212		 , 0.001935323	   , 0.3307985		},
	{ 0.07763		 , 0.00218		   , 0.3713			},
	{ 0.08695811	 , 0.0024548	   , 0.4162091		},
	{ 0.09717672	 , 0.002764		   , 0.4654642		},
	{ 0.1084063		 , 0.0031178	   , 0.5196948		},
	{ 0.1207672		 , 0.0035264	   , 0.5795303		},
	{ 0.13438		 , 0.004		   , 0.6456			},
	{ 0.1493582		 , 0.00454624	   , 0.7184838		},
	{ 0.1653957		 , 0.00515932	   , 0.7967133		},
	{ 0.1819831		 , 0.00582928	   , 0.8778459		},
	{ 0.198611		 , 0.00654616	   , 0.959439		},
	{ 0.21477		 , 0.0073		   , 1.0390501		},
	{ 0.2301868		 , 0.008086507	   , 1.1153673		},
	{ 0.2448797		 , 0.00890872	   , 1.1884971		},
	{ 0.2587773		 , 0.00976768	   , 1.2581233		},
	{ 0.2718079		 , 0.01066443	   , 1.3239296		},
	{ 0.2839		 , 0.0116		   , 1.3856			},
	{ 0.2949438		 , 0.01257317	   , 1.4426352		},
	{ 0.3048965		 , 0.01358272	   , 1.4948035		},
	{ 0.3137873		 , 0.01462968	   , 1.5421903		},
	{ 0.3216454		 , 0.01571509	   , 1.5848807		},
	{ 0.3285		 , 0.01684		   , 1.62296		},
	{ 0.3343513		 , 0.01800736	   , 1.6564048		},
	{ 0.3392101		 , 0.01921448	   , 1.6852959		},
	{ 0.3431213		 , 0.02045392	   , 1.7098745		},
	{ 0.3461296		 , 0.02171824	   , 1.7303821		},
	{ 0.34828		 , 0.023		   , 1.74706		},
	{ 0.3495999		 , 0.02429461	   , 1.7600446		},
	{ 0.3501474		 , 0.02561024	   , 1.7696233		},
	{ 0.350013		 , 0.02695857	   , 1.7762637		},
	{ 0.349287		 , 0.02835125	   , 1.7804334		},
	{ 0.34806		 , 0.0298		   , 1.7826			},
	{ 0.3463733		 , 0.03131083	   , 1.7829682		},
	{ 0.3442624		 , 0.03288368	   , 1.7816998		},
	{ 0.3418088		 , 0.03452112	   , 1.7791982		},
	{ 0.3390941		 , 0.03622571	   , 1.7758671		},
	{ 0.3362		 , 0.038		   , 1.77211		},
	{ 0.3331977		 , 0.03984667	   , 1.7682589		},
	{ 0.3300411		 , 0.041768		   , 1.764039		},
	{ 0.3266357		 , 0.043766		   , 1.7589438		},
	{ 0.3228868		 , 0.04584267	   , 1.7524663		},
	{ 0.3187		 , 0.048		   , 1.7441			},
	{ 0.3140251		 , 0.05024368	   , 1.7335595		},
	{ 0.308884		 , 0.05257304	   , 1.7208581		},
	{ 0.3032904		 , 0.05498056	   , 1.7059369		},
	{ 0.2972579		 , 0.05745872	   , 1.6887372		},
	{ 0.2908		 , 0.06			   , 1.6692			},
	{ 0.2839701		 , 0.06260197	   , 1.6475287		},
	{ 0.2767214		 , 0.06527752	   , 1.6234127		},
	{ 0.2689178		 , 0.06804208	   , 1.5960223		},
	{ 0.2604227		 , 0.07091109	   , 1.564528		},
	{ 0.2511		 , 0.0739		   , 1.5281			},
	{ 0.2408475		 , 0.077016		   , 1.4861114		},
	{ 0.2298512		 , 0.0802664	   , 1.4395215		},
	{ 0.2184072		 , 0.0836668	   , 1.3898799		},
	{ 0.2068115		 , 0.0872328	   , 1.3387362		},
	{ 0.19536		 , 0.09098		   , 1.28764		},
	{ 0.1842136		 , 0.09491755	   , 1.2374223		},
	{ 0.1733273		 , 0.09904584	   , 1.1878243		},
	{ 0.1626881		 , 0.1033674	   , 1.1387611		},
	{ 0.1522833		 , 0.1078846	   , 1.090148		},
	{ 0.1421		 , 0.1126		   , 1.0419			},
	{ 0.1321786		 , 0.117532		   , 0.9941976		},
	{ 0.1225696		 , 0.1226744	   , 0.9473473		},
	{ 0.1132752		 , 0.1279928	   , 0.9014531		},
	{ 0.1042979		 , 0.1334528	   , 0.8566193		},
	{ 0.09564		 , 0.13902		   , 0.8129501		},
	{ 0.08729955	 , 0.1446764	   , 0.7705173		},
	{ 0.07930804	 , 0.1504693	   , 0.7294448		},
	{ 0.07171776	 , 0.1564619	   , 0.6899136		},
	{ 0.06458099	 , 0.1627177	   , 0.6521049		},
	{ 0.05795001	 , 0.1693		   , 0.6162			},
	{ 0.05186211	 , 0.1762431	   , 0.5823286		},
	{ 0.04628152	 , 0.1835581	   , 0.5504162		},
	{ 0.04115088	 , 0.1912735	   , 0.5203376		},
	{ 0.03641283	 , 0.199418		   , 0.4919673		},
	{ 0.03201		 , 0.20802		   , 0.46518		},
	{ 0.0279172		 , 0.2171199	   , 0.4399246		},
	{ 0.0241444		 , 0.2267345	   , 0.4161836		},
	{ 0.020687		 , 0.2368571	   , 0.3938822		},
	{ 0.0175404		 , 0.2474812	   , 0.3729459		},
	{ 0.0147		 , 0.2586		   , 0.3533			},
	{ 0.01216179	 , 0.2701849	   , 0.3348578		},
	{ 0.00991996	 , 0.2822939	   , 0.3175521		},
	{ 0.00796724	 , 0.2950505	   , 0.3013375		},
	{ 0.006296346	 , 0.308578		   , 0.2861686		},
	{ 0.0049		 , 0.323		   , 0.272			},
	{ 0.003777173	 , 0.3384021	   , 0.2588171		},
	{ 0.00294532	 , 0.3546858	   , 0.2464838		},
	{ 0.00242488	 , 0.3716986	   , 0.2347718		},
	{ 0.002236293	 , 0.3892875	   , 0.2234533		},
	{ 0.0024		 , 0.4073		   , 0.2123			},
	{ 0.00292552	 , 0.4256299	   , 0.2011692		},
	{ 0.00383656	 , 0.4443096	   , 0.1901196		},
	{ 0.00517484	 , 0.4633944	   , 0.1792254		},
	{ 0.00698208	 , 0.4829395	   , 0.1685608		},
	{ 0.0093		 , 0.503		   , 0.1582			},
	{ 0.01214949	 , 0.5235693	   , 0.1481383		},
	{ 0.01553588	 , 0.544512		   , 0.1383758		},
	{ 0.01947752	 , 0.56569		   , 0.1289942		},
	{ 0.02399277	 , 0.5869653	   , 0.1200751		},
	{ 0.0291		 , 0.6082		   , 0.1117			},
	{ 0.03481485	 , 0.6293456	   , 0.1039048		},
	{ 0.04112016	 , 0.6503068	   , 0.09666748		},
	{ 0.04798504	 , 0.6708752	   , 0.08998272		},
	{ 0.05537861	 , 0.6908424	   , 0.08384531		},
	{ 0.06327		 , 0.71			   , 0.07824999		},
	{ 0.07163501	 , 0.7281852	   , 0.07320899		},
	{ 0.08046224	 , 0.7454636	   , 0.06867816		},
	{ 0.08973996	 , 0.7619694	   , 0.06456784		},
	{ 0.09945645	 , 0.7778368	   , 0.06078835		},
	{ 0.1096		 , 0.7932		   , 0.05725001		},
	{ 0.1201674		 , 0.8081104	   , 0.05390435		},
	{ 0.1311145		 , 0.8224962	   , 0.05074664		},
	{ 0.1423679		 , 0.8363068	   , 0.04775276		},
	{ 0.1538542		 , 0.8494916	   , 0.04489859		},
	{ 0.1655		 , 0.862		   , 0.04216		},
	{ 0.1772571		 , 0.8738108	   , 0.03950728		},
	{ 0.18914		 , 0.8849624	   , 0.03693564		},
	{ 0.2011694		 , 0.8954936	   , 0.03445836		},
	{ 0.2133658		 , 0.9054432	   , 0.03208872		},
	{ 0.2257499		 , 0.9148501	   , 0.02984		},
	{ 0.2383209		 , 0.9237348	   , 0.02771181		},
	{ 0.2510668		 , 0.9320924	   , 0.02569444		},
	{ 0.2639922		 , 0.9399226	   , 0.02378716		},
	{ 0.2771017		 , 0.9472252	   , 0.02198925		},
	{ 0.2904		 , 0.954		   , 0.0203			},
	{ 0.3038912		 , 0.9602561	   , 0.01871805		},
	{ 0.3175726		 , 0.9660074	   , 0.01724036		},
	{ 0.3314384		 , 0.9712606	   , 0.01586364		},
	{ 0.3454828		 , 0.9760225	   , 0.01458461		},
	{ 0.3597		 , 0.9803		   , 0.0134			},
	{ 0.3740839		 , 0.9840924	   , 0.01230723		},
	{ 0.3886396		 , 0.9874182	   , 0.01130188		},
	{ 0.4033784		 , 0.9903128	   , 0.01037792		},
	{ 0.4183115		 , 0.9928116	   , 0.009529306	},
	{ 0.4334499		 , 0.9949501	   , 0.008749999	},
	{ 0.4487953		 , 0.9967108	   , 0.0080352		},
	{ 0.464336		 , 0.9980983	   , 0.0073816		},
	{ 0.480064		 , 0.999112		   , 0.0067854		},
	{ 0.4959713		 , 0.9997482	   , 0.0062428		},
	{ 0.5120501		 , 1			   , 0.005749999	},
	{ 0.5282959		 , 0.9998567	   , 0.0053036		},
	{ 0.5446916		 , 0.9993046	   , 0.0048998		},
	{ 0.5612094		 , 0.9983255	   , 0.0045342		},
	{ 0.5778215		 , 0.9968987	   , 0.0042024		},
	{ 0.5945		 , 0.995		   , 0.0039			},
	{ 0.6112209		 , 0.9926005	   , 0.0036232		},
	{ 0.6279758		 , 0.9897426	   , 0.0033706		},
	{ 0.6447602		 , 0.9864444	   , 0.0031414		},
	{ 0.6615697		 , 0.9827241	   , 0.0029348		},
	{ 0.6784		 , 0.9786		   , 0.002749999	},
	{ 0.6952392		 , 0.9740837	   , 0.0025852		},
	{ 0.7120586		 , 0.9691712	   , 0.0024386		},
	{ 0.7288284		 , 0.9638568	   , 0.0023094		},
	{ 0.7455188		 , 0.9581349	   , 0.0021968		},
	{ 0.7621		 , 0.952		   , 0.0021			},
	{ 0.7785432		 , 0.9454504	   , 0.002017733	},
	{ 0.7948256		 , 0.9384992	   , 0.0019482		},
	{ 0.8109264		 , 0.9311628	   , 0.0018898		},
	{ 0.8268248		 , 0.9234576	   , 0.001840933	},
	{ 0.8425		 , 0.9154		   , 0.0018			},
	{ 0.8579325		 , 0.9070064	   , 0.001766267	},
	{ 0.8730816		 , 0.8982772	   , 0.0017378		},
	{ 0.8878944		 , 0.8892048	   , 0.0017112		},
	{ 0.9023181		 , 0.8797816	   , 0.001683067	},
	{ 0.9163		 , 0.87			   , 0.001650001	},
	{ 0.9297995		 , 0.8598613	   , 0.001610133	},
	{ 0.9427984		 , 0.849392		   , 0.0015644		},
	{ 0.9552776		 , 0.838622		   , 0.0015136		},
	{ 0.9672179		 , 0.8275813	   , 0.001458533	},
	{ 0.9786		 , 0.8163		   , 0.0014			},
	{ 0.9893856		 , 0.8047947	   , 0.001336667	},
	{ 0.9995488		 , 0.793082		   , 0.00127		},
	{ 1.0090892		 , 0.781192		   , 0.001205		},
	{ 1.0180064		 , 0.7691547	   , 0.001146667	},
	{ 1.0263		 , 0.757		   , 0.0011			},
	{ 1.0339827		 , 0.7447541	   , 0.0010688		},
	{ 1.040986		 , 0.7324224	   , 0.0010494		},
	{ 1.047188		 , 0.7200036	   , 0.0010356		},
	{ 1.0524667		 , 0.7074965	   , 0.0010212		},
	{ 1.0567		 , 0.6949		   , 0.001			},
	{ 1.0597944		 , 0.6822192	   , 0.00096864		},
	{ 1.0617992		 , 0.6694716	   , 0.00092992		},
	{ 1.0628068		 , 0.6566744	   , 0.00088688		},
	{ 1.0629096		 , 0.6438448	   , 0.00084256		},
	{ 1.0622		 , 0.631		   , 0.0008			},
	{ 1.0607352		 , 0.6181555	   , 0.00076096		},
	{ 1.0584436		 , 0.6053144	   , 0.00072368		},
	{ 1.0552244		 , 0.5924756	   , 0.00068592		},
	{ 1.0509768		 , 0.5796379	   , 0.00064544		},
	{ 1.0456		 , 0.5668		   , 0.0006			},
	{ 1.0390369		 , 0.5539611	   , 0.0005478667	},
	{ 1.0313608		 , 0.5411372	   , 0.0004916		},
	{ 1.0226662		 , 0.5283528	   , 0.0004354		},
	{ 1.0130477		 , 0.5156323	   , 0.0003834667	},
	{ 1.0026		 , 0.503		   , 0.00034		},
	{ 0.9913675		 , 0.4904688	   , 0.0003072533	},
	{ 0.9793314		 , 0.4780304	   , 0.00028316		},
	{ 0.9664916		 , 0.4656776	   , 0.00026544		},
	{ 0.9528479		 , 0.4534032	   , 0.0002518133	},
	{ 0.9384		 , 0.4412		   , 0.00024		},
	{ 0.923194		 , 0.42908		   , 0.0002295467	},
	{ 0.907244		 , 0.417036		   , 0.00022064		},
	{ 0.890502		 , 0.405032		   , 0.00021196		},
	{ 0.87292		 , 0.393032		   , 0.0002021867	},
	{ 0.8544499		 , 0.381		   , 0.00019		},
	{ 0.835084		 , 0.3689184	   , 0.0001742133	},
	{ 0.814946		 , 0.3568272	   , 0.00015564		},
	{ 0.794186		 , 0.3447768	   , 0.00013596		},
	{ 0.772954		 , 0.3328176	   , 0.0001168533	},
	{ 0.7514		 , 0.321		   , 0.0001			},
	{ 0.7295836		 , 0.3093381	   , 0.00008613333	},
	{ 0.7075888		 , 0.2978504	   , 0.0000746		},
	{ 0.6856022		 , 0.2865936	   , 0.000065		},
	{ 0.6638104		 , 0.2756245	   , 0.00005693333	},
	{ 0.6424		 , 0.265		   , 0.00004999999	},
	{ 0.6215149		 , 0.2547632	   , 0.00004416		},
	{ 0.6011138		 , 0.2448896	   , 0.00003948		},
	{ 0.5811052		 , 0.2353344	   , 0.00003572		},
	{ 0.5613977		 , 0.2260528	   , 0.00003264		},
	{ 0.5419		 , 0.217		   , 0.00003		},
	{ 0.5225995		 , 0.2081616	   , 0.00002765333	},
	{ 0.5035464		 , 0.1995488	   , 0.00002556		},
	{ 0.4847436		 , 0.1911552	   , 0.00002364		},
	{ 0.4661939		 , 0.1829744	   , 0.00002181333	},
	{ 0.4479		 , 0.175		   , 0.00002		},
	{ 0.4298613		 , 0.1672235	   , 0.00001813333	},
	{ 0.412098		 , 0.1596464	   , 0.0000162		},
	{ 0.394644		 , 0.1522776	   , 0.0000142		},
	{ 0.3775333		 , 0.1451259	   , 0.00001213333	},
	{ 0.3608		 , 0.1382		   , 0.00001		},
	{ 0.3444563		 , 0.1315003	   , 0.000007733333 },
	{ 0.3285168		 , 0.1250248	   , 0.0000054		},
	{ 0.3130192		 , 0.1187792	   , 0.0000032		},
	{ 0.2980011		 , 0.1127691	   , 0.000001333333 },
	{ 0.2835		 , 0.107		   , 0				},
	{ 0.2695448		 , 0.1014762	   , 0				},
	{ 0.2561184		 , 0.09618864	   , 0				},
	{ 0.2431896		 , 0.09112296	   , 0				},
	{ 0.2307272		 , 0.08626485	   , 0				},
	{ 0.2187		 , 0.0816		   , 0				},
	{ 0.2070971		 , 0.07712064	   , 0				},
	{ 0.1959232		 , 0.07282552	   , 0				},
	{ 0.1851708		 , 0.06871008	   , 0				},
	{ 0.1748323		 , 0.06476976	   , 0				},
	{ 0.1649		 , 0.061		   , 0				},
	{ 0.1553667		 , 0.05739621	   , 0				},
	{ 0.14623		 , 0.05395504	   , 0				},
	{ 0.13749		 , 0.05067376	   , 0				},
	{ 0.1291467		 , 0.04754965	   , 0				},
	{ 0.1212		 , 0.04458		   , 0				},
	{ 0.1136397		 , 0.04175872	   , 0				},
	{ 0.106465		 , 0.03908496	   , 0				},
	{ 0.09969044	 , 0.03656384	   , 0				},
	{ 0.09333061	 , 0.03420048	   , 0				},
	{ 0.0874		 , 0.032		   , 0				},
	{ 0.08190096	 , 0.02996261	   , 0				},
	{ 0.07680428	 , 0.02807664	   , 0				},
	{ 0.07207712	 , 0.02632936	   , 0				},
	{ 0.06768664	 , 0.02470805	   , 0				},
	{ 0.0636		 , 0.0232		   , 0				},
	{ 0.05980685	 , 0.02180077	   , 0				},
	{ 0.05628216	 , 0.02050112	   , 0				},
	{ 0.05297104	 , 0.01928108	   , 0				},
	{ 0.04981861	 , 0.01812069	   , 0				},
	{ 0.04677		 , 0.017		   , 0				},
	{ 0.04378405	 , 0.01590379	   , 0				},
	{ 0.04087536	 , 0.01483718	   , 0				},
	{ 0.03807264	 , 0.01381068	   , 0				},
	{ 0.03540461	 , 0.01283478	   , 0				},
	{ 0.0329		 , 0.01192		   , 0				},
	{ 0.03056419	 , 0.01106831	   , 0				},
	{ 0.02838056	 , 0.01027339	   , 0				},
	{ 0.02634484	 , 0.009533311	   , 0				},
	{ 0.02445275	 , 0.008846157	   , 0				},
	{ 0.0227		 , 0.00821		   , 0				},
	{ 0.02108429	 , 0.007623781	   , 0				},
	{ 0.01959988	 , 0.007085424	   , 0				},
	{ 0.01823732	 , 0.006591476	   , 0				},
	{ 0.01698717	 , 0.006138485	   , 0				},
	{ 0.01584		 , 0.005723		   , 0				},
	{ 0.01479064	 , 0.005343059	   , 0				},
	{ 0.01383132	 , 0.004995796	   , 0				},
	{ 0.01294868	 , 0.004676404	   , 0				},
	{ 0.0121292		 , 0.004380075	   , 0				},
	{ 0.01135916	 , 0.004102		   , 0				},
	{ 0.01062935	 , 0.003838453	   , 0				},
	{ 0.009938846	 , 0.003589099	   , 0				},
	{ 0.009288422	 , 0.003354219	   , 0				},
	{ 0.008678854	 , 0.003134093	   , 0				},
	{ 0.008110916	 , 0.002929		   , 0				},
	{ 0.007582388	 , 0.002738139	   , 0				},
	{ 0.007088746	 , 0.002559876	   , 0				},
	{ 0.006627313	 , 0.002393244	   , 0				},
	{ 0.006195408	 , 0.002237275	   , 0				},
	{ 0.005790346	 , 0.002091		   , 0				},
	{ 0.005409826	 , 0.001953587	   , 0				},
	{ 0.005052583	 , 0.00182458	   , 0				},
	{ 0.004717512	 , 0.00170358	   , 0				},
	{ 0.004403507	 , 0.001590187	   , 0				},
	{ 0.004109457	 , 0.001484		   , 0				},
	{ 0.003833913	 , 0.001384496	   , 0				},
	{ 0.003575748	 , 0.001291268	   , 0				},
	{ 0.003334342	 , 0.001204092	   , 0				},
	{ 0.003109075	 , 0.001122744	   , 0				},
	{ 0.002899327	 , 0.001047		   , 0				},
	{ 0.002704348	 , 0.0009765896	   , 0				},
	{ 0.00252302	 , 0.0009111088	   , 0				},
	{ 0.002354168	 , 0.0008501332	   , 0				},
	{ 0.002196616	 , 0.0007932384	   , 0				},
	{ 0.00204919	 , 0.00074		   , 0				},
	{ 0.00191096	 , 0.0006900827	   , 0				},
	{ 0.001781438	 , 0.00064331	   , 0				},
	{ 0.00166011	 , 0.000599496	   , 0				},
	{ 0.001546459	 , 0.0005584547	   , 0				},
	{ 0.001439971	 , 0.00052		   , 0				},
	{ 0.001340042	 , 0.0004839136	   , 0				},
	{ 0.001246275	 , 0.0004500528	   , 0				},
	{ 0.001158471	 , 0.0004183452	   , 0				},
	{ 0.00107643	 , 0.0003887184	   , 0				},
	{ 0.0009999493	 , 0.0003611	   , 0				},
	{ 0.0009287358	 , 0.0003353835	   , 0				},
	{ 0.0008624332	 , 0.0003114404	   , 0				},
	{ 0.0008007503	 , 0.0002891656	   , 0				},
	{ 0.000743396	 , 0.0002684539	   , 0				},
	{ 0.0006900786	 , 0.0002492	   , 0				},
	{ 0.0006405156	 , 0.0002313019	   , 0				},
	{ 0.0005945021	 , 0.0002146856	   , 0				},
	{ 0.0005518646	 , 0.0001992884	   , 0				},
	{ 0.000512429	 , 0.0001850475	   , 0				},
	{ 0.0004760213	 , 0.0001719	   , 0				},
	{ 0.0004424536	 , 0.0001597781	   , 0				},
	{ 0.0004115117	 , 0.0001486044	   , 0				},
	{ 0.0003829814	 , 0.0001383016	   , 0				},
	{ 0.0003566491	 , 0.0001287925	   , 0				},
	{ 0.0003323011	 , 0.00012		   , 0				},
	{ 0.0003097586	 , 0.0001118595	   , 0				},
	{ 0.0002888871	 , 0.0001043224	   , 0				},
	{ 0.0002695394	 , 0.0000973356	   , 0				},
	{ 0.0002515682	 , 0.00009084587   , 0				},
	{ 0.0002348261	 , 0.0000848	   , 0				},
	{ 0.000219171	 , 0.00007914667   , 0				},
	{ 0.0002045258	 , 0.000073858	   , 0				},
	{ 0.0001908405	 , 0.000068916	   , 0				},
	{ 0.0001780654	 , 0.00006430267   , 0				},
	{ 0.0001661505	 , 0.00006		   , 0				},
	{ 0.0001550236	 , 0.00005598187   , 0				},
	{ 0.0001446219	 , 0.0000522256	   , 0				},
	{ 0.0001349098	 , 0.0000487184	   , 0				},
	{ 0.000125852	 , 0.00004544747   , 0				},
	{ 0.000117413	 , 0.0000424	   , 0				},
	{ 0.0001095515	 , 0.00003956104   , 0				},
	{ 0.0001022245	 , 0.00003691512   , 0				},
	{ 0.00009539445	 , 0.00003444868   , 0				},
	{ 0.0000890239	 , 0.00003214816   , 0				},
	{ 0.00008307527	 , 0.00003		   , 0				},
	{ 0.00007751269	 , 0.00002799125   , 0				},
	{ 0.00007231304	 , 0.00002611356   , 0				},
	{ 0.00006745778	 , 0.00002436024   , 0				},
	{ 0.00006292844	 , 0.00002272461   , 0				},
	{ 0.00005870652	 , 0.0000212	   , 0				},
	{ 0.00005477028	 , 0.00001977855   , 0				},
	{ 0.00005109918	 , 0.00001845285   , 0				},
	{ 0.00004767654	 , 0.00001721687   , 0				},
	{ 0.00004448567	 , 0.00001606459   , 0				},
	{ 0.00004150994	 , 0.00001499	   , 0				},
	{ 0.00003873324	 , 0.00001398728   , 0				},
	{ 0.00003614203	 , 0.00001305155   , 0				},
	{ 0.00003372352	 , 0.00001217818   , 0				},
	{ 0.00003146487	 , 0.00001136254   , 0				},
	{ 0.00002935326	 , 0.0000106	   , 0				},
	{ 0.00002737573	 , 0.000009885877  , 0				},
	{ 0.00002552433	 , 0.000009217304  , 0				},
	{ 0.00002379376	 , 0.000008592362  , 0				},
	{ 0.0000221787	 , 0.000008009133  , 0				},
	{ 0.00002067383	 , 0.0000074657	   , 0				},
	{ 0.00001927226	 , 0.000006959567  , 0				},
	{ 0.0000179664	 , 0.000006487995  , 0				},
	{ 0.00001674991	 , 0.000006048699  , 0				},
	{ 0.00001561648	 , 0.000005639396  , 0				},
	{ 0.00001455977	 , 0.0000052578	   , 0				},
	{ 0.00001357387	 , 0.000004901771  , 0				},
	{ 0.00001265436	 , 0.00000456972   , 0				},
	{ 0.00001179723	 , 0.000004260194  , 0				},
	{ 0.00001099844	 , 0.000003971739  , 0				},
	{ 0.00001025398	 , 0.0000037029	   , 0				},
	{ 0.000009559646 , 0.000003452163  , 0				},
	{ 0.000008912044 , 0.000003218302  , 0				},
	{ 0.000008308358 , 0.0000030003	   , 0				},
	{ 0.000007745769 , 0.000002797139  , 0				},
	{ 0.000007221456 , 0.0000026078	   , 0				},
	{ 0.000006732475 , 0.00000243122   , 0				},
	{ 0.000006276423 , 0.000002266531  , 0				},
	{ 0.000005851304 , 0.000002113013  , 0				},
	{ 0.000005455118 , 0.000001969943  , 0				},
	{ 0.000005085868 , 0.0000018366	   , 0				},
	{ 0.000004741466 , 0.00000171223   , 0				},
	{ 0.000004420236 , 0.000001596228  , 0				},
	{ 0.000004120783 , 0.00000148809   , 0				},
	{ 0.000003841716 , 0.000001387314  , 0				},
	{ 0.000003581652 , 0.0000012934	   , 0				},
	{ 0.000003339127 , 0.00000120582   , 0				},
	{ 0.000003112949 , 0.000001124143  , 0				},
	{ 0.000002902121 , 0.000001048009  , 0				},
	{ 0.000002705645 , 0.0000009770578 , 0				},
	{ 0.000002522525 , 0.00000091093   , 0				},
	{ 0.000002351726 , 0.0000008492513 , 0				},
	{ 0.000002192415 , 0.0000007917212 , 0				},
	{ 0.000002043902 , 0.0000007380904 , 0				},
	{ 0.000001905497 , 0.0000006881098 , 0				},
	{ 0.000001776509 , 0.00000064153   , 0				},
	{ 0.000001656215 , 0.0000005980895 , 0				},
	{ 0.000001544022 , 0.0000005575746 , 0				},
	{ 0.00000143944	 , 0.000000519808  , 0				},
	{ 0.000001341977 , 0.0000004846123 , 0				},
	{ 0.000001251141 , 0.00000045181   , 0				},
	};

/*****************************************************************************/

void dng_illuminant_data::CalculateSpectrumXY ()
	{

	// Make piecewise linear function from the data.

	dng_piecewise_linear spectrumFunc;

	spectrumFunc.X.reserve (fSpectrum.size ());
	spectrumFunc.Y.reserve (fSpectrum.size ());

	real64 minLambda64 = fMinLambda.As_real64 ();

	real64 lambdaStep64 = fLambdaSpacing.As_real64 ();

	real64 maxLambda64 = minLambda64;
	
	for (size_t i = 0; i < fSpectrum.size (); i++)
		{

		real64 lambda = minLambda64 + (i * lambdaStep64);

		spectrumFunc.Add (lambda, fSpectrum [i].As_real64 ());

		maxLambda64 = lambda;
		
		}

	// Check std observer data size.

	const size_t numObserverSamples = 830 - 360 + 1;

	DNG_REQUIRE (numObserverSamples ==
				 (sizeof ( kCIEStdObserver2Degree) /
				  sizeof (*kCIEStdObserver2Degree)),
				 "Mismatch number of CIE std observer samples");

	// Find XYZ by summing from 360 to 830 nm in 1-nm steps.

	dng_vector_3 sum;

	dng_vector_3 observerSum;

	for (int32 i = 360; i <= 830; i++)
		{

		int32 observerIndex = i - 360;
		
		real64 lambda = (real64) i;

		lambda = Pin_real64 (minLambda64, lambda, maxLambda64);

		real64 light = spectrumFunc.Evaluate (lambda);

		real64 x = kCIEStdObserver2Degree [observerIndex] [0];
		real64 y = kCIEStdObserver2Degree [observerIndex] [1];
		real64 z = kCIEStdObserver2Degree [observerIndex] [2];

		observerSum [0] += x;
		observerSum [1] += y;
		observerSum [2] += z;
		
		sum [0] += (x * light);
		sum [1] += (y * light);
		sum [2] += (z * light);
		
		}

	// Ideally the sums of the kCIEStdObserver2Degree columns should all be
	// the same, but due to rounding they are not. So just divide out the
	// observer sums.

	sum [0] = sum [0] / observerSum [0];
	sum [1] = sum [1] / observerSum [1];
	sum [2] = sum [2] / observerSum [2];

	if (sum.MinEntry () > 0.0)
		{

		fDerivedWhite = XYZtoXY (sum);
		
		}

	else
		{

		ThrowBadFormat ("invalid spectrum-derived white point");

		}

	}

/*****************************************************************************/

class dng_map_temp_func: public dng_1d_function
	{
		
	public:

		real64 Evaluate (real64 x) const override
			{

			// x is temperature

			// map to 1/temperature, and map 1/1500 to 1.0.

			return Min_real64 (1500.0 / x, 1.0);

			};
		
	};

/*****************************************************************************/

void CalculateTripleIlluminantWeights (const dng_xy_coord &white,
									   const dng_illuminant_data &light1,
									   const dng_illuminant_data &light2,
									   const dng_illuminant_data &light3,
									   real64 &out_w1,
									   real64 &out_w2,
									   real64 &out_w3)
	{

	// Compute distance from white to each of the lights in a scaled
	// (1/temperature, tint) space, then map the distances to weights.

	const dng_xy_coord &white1 = light1.WhiteXY ();
	const dng_xy_coord &white2 = light2.WhiteXY ();
	const dng_xy_coord &white3 = light3.WhiteXY ();

	dng_temperature tt (white);

	dng_temperature tt1 (white1);
	dng_temperature tt2 (white2);
	dng_temperature tt3 (white3);

	dng_point_real64 pt	 (tt .Tint (), tt .Temperature ());
	dng_point_real64 pt1 (tt1.Tint (), tt1.Temperature ());
	dng_point_real64 pt2 (tt2.Tint (), tt2.Temperature ());
	dng_point_real64 pt3 (tt3.Tint (), tt3.Temperature ());

	// Map the tints.

	const real64 kTintScale = 1.0 / 200.0;

	pt .v *= kTintScale;
	pt1.v *= kTintScale;
	pt2.v *= kTintScale;
	pt3.v *= kTintScale;

	// Map the temperatures.

	dng_map_temp_func tempMap;

	pt .h = tempMap.Evaluate (pt .h);
	pt1.h = tempMap.Evaluate (pt1.h);
	pt2.h = tempMap.Evaluate (pt2.h);
	pt3.h = tempMap.Evaluate (pt3.h);

	// Compute squared distances and convert to weights.

	real64 w1 = DistanceSquared (pt, pt1);
	real64 w2 = DistanceSquared (pt, pt2);
	real64 w3 = DistanceSquared (pt, pt3);

	const real64 kMinDist = 1.0e-8;

	w1 = 1.0 / (w1 + kMinDist);
	w2 = 1.0 / (w2 + kMinDist);
	w3 = 1.0 / (w3 + kMinDist);

	// Normalize.

	real64 sum = w1 + w2 + w3;

	w1 /= sum;
	w2 /= sum;
	w3 /= sum;

	// Smooth.

	w1 = SmoothStep (w1);
	w2 = SmoothStep (w2);
	w3 = SmoothStep (w3);

	// Suppress small weights.

	// Map kMinWeight to 0.
	// Map 1 to 1.
	// Clip result to [0,1].

	const real64 kMinWeight = 0.02;

	const real64 scale = 1.0 / (1.0 - kMinWeight);

	w1 = Pin_real64 ((w1 - kMinWeight) * scale);
	w2 = Pin_real64 ((w2 - kMinWeight) * scale);
	w3 = Pin_real64 ((w3 - kMinWeight) * scale);

	// Renormalize.

	sum = w1 + w2 + w3;

	w1 /= sum;
	w2 /= sum;

	// Store.

	out_w1 = w1;
	out_w2 = w2;
	out_w3 = Max_real64 (1.0 - w1 - w2, 0.0);

	}

/*****************************************************************************/
