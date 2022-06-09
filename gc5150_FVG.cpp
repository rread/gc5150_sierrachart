#include "sierrachart.h"
#include <vector>
#include <algorithm>
SCDLLName("gc5150 - FVG")
const SCString ContactInformation = "gc5150, @gc5150 (twitter)";

/*==============================================================================
	This study is for drawing FVG's (Fair Value Gaps)
------------------------------------------------------------------------------*/
SCSFExport scsf_FVG(SCStudyInterfaceRef sc)
{
	// Input Index
	int SCInputIndex = 0;

	// FVG Up Settings
	SCInputRef Input_FVGUpEnabled = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpLineWidth = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpLineColor = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpFillColor = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpTransparencyLevel = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpDrawMidline = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpExtendRight = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpDeleteWhenFilled = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpMinGapSizeInTicks = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGUpAllowCopyToOtherCharts = sc.Input[SCInputIndex++];
	
	// FVG Down Settings
	SCInputRef Input_FVGDnEnabled = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnLineWidth = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnLineColor = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnFillColor = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnTransparencyLevel = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnDrawMidline = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnExtendRight = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnDeleteWhenFilled = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnMinGapSizeInTicks = sc.Input[SCInputIndex++];
	SCInputRef Input_FVGDnAllowCopyToOtherCharts = sc.Input[SCInputIndex++];

	// General Settings
	SCInputRef Input_FVGMaxBarLookback = sc.Input[SCInputIndex++];

	const int MIN_START_INDEX = 2; // Need at least 3 bars [0,1,2]

	struct FVGRectangle {
		int LineNumber;
		int ToolBeginIndex;
		float ToolBeginValue;
		int ToolEndIndex;
		float ToolEndValue;
		bool FVGEnded;
		bool FVGUp; // If not up, then it's down
	};

	struct HLForBarIndex {
		int Index;
		float High;
		float Low;
	};

	if (sc.SetDefaults)
	{
		sc.GraphName = "FVG";
		SCString studyDescription;
		studyDescription.Format("%s by %s\nThis study draws Fair Value Gaps", sc.GraphName.GetChars(), ContactInformation.GetChars());
		sc.StudyDescription = studyDescription;
		sc.GraphRegion = 0;
		sc.AutoLoop = 0;

		// FVG Up
		Input_FVGUpEnabled.Name = "FVG Up: Enabled";
		Input_FVGUpEnabled.SetDescription("Draw FVG Up Gaps");
		Input_FVGUpEnabled.SetYesNo(1);

		Input_FVGUpLineWidth.Name = "FVG Up: Line Width";
		Input_FVGUpLineWidth.SetDescription("Width of FVG Rectangle Border");
		Input_FVGUpLineWidth.SetInt(0);

		Input_FVGUpLineColor.Name = "FVG Up: Line Color";
		Input_FVGUpLineColor.SetColor(RGB(13, 166, 240));
		Input_FVGUpLineColor.SetDescription("Color of FVG Rectangle Border");

		Input_FVGUpFillColor.Name = "FVG Up: Fill Color";
		Input_FVGUpFillColor.SetDescription("Fill Color Used for FVG Rectangle");
		Input_FVGUpFillColor.SetColor(RGB(13, 166, 240));

		Input_FVGUpTransparencyLevel.Name = "FVG Up: Transparency Level";
		Input_FVGUpTransparencyLevel.SetDescription("Transparency Level for FVG Rectangle Fill");
		Input_FVGUpTransparencyLevel.SetInt(65);
		Input_FVGUpTransparencyLevel.SetIntLimits(0, 100);

		Input_FVGUpDrawMidline.Name = "FVG Up: Draw Midline (Set Line Width to 1 or Higher)";
		Input_FVGUpDrawMidline.SetDescription("Draw Midline for FVG Rectangle. Requires Line Width of 1 or Higher.");
		Input_FVGUpDrawMidline.SetYesNo(0);

		Input_FVGUpExtendRight.Name = "FVG Up: Extend Right";
		Input_FVGUpExtendRight.SetDescription("Extend FVG Rectangle to Right of Chart Until Filled");
		Input_FVGUpExtendRight.SetYesNo(1);

		Input_FVGUpDeleteWhenFilled.Name = "FVG Up: Delete When Filled";
		Input_FVGUpDeleteWhenFilled.SetDescription("Hide FVG Rectangle if Gap is Filled");
		Input_FVGUpDeleteWhenFilled.SetYesNo(1);

		Input_FVGUpMinGapSizeInTicks.Name = "FVG Up: Minimum Gap Size in Ticks";
		Input_FVGUpMinGapSizeInTicks.SetDescription("Only Process Gaps if greater or equal to Specified Gap Size");
		Input_FVGUpMinGapSizeInTicks.SetInt(1);
		Input_FVGUpMinGapSizeInTicks.SetIntLimits(1, INT_MAX);
		
		Input_FVGUpAllowCopyToOtherCharts.Name = "FVG Up: Allow Copy To Other Charts";
		Input_FVGUpAllowCopyToOtherCharts.SetDescription("Allow the FVG Rectangles to be Copied to Other Charts");
		Input_FVGUpAllowCopyToOtherCharts.SetYesNo(0);

		// FVG Down
		Input_FVGDnEnabled.Name = "FVG Down: Enabled";
		Input_FVGDnEnabled.SetDescription("Draw FVG Down Gaps");
		Input_FVGDnEnabled.SetYesNo(1);

		Input_FVGDnLineWidth.Name = "FVG Down: Line Width";
		Input_FVGDnLineWidth.SetDescription("Width of Rectangle Border");
		Input_FVGDnLineWidth.SetInt(0);

		Input_FVGDnLineColor.Name = "FVG Down: Line Color";
		Input_FVGDnLineColor.SetDescription("Color of Rectangle Border");
		Input_FVGDnLineColor.SetColor(RGB(255, 128, 128));

		Input_FVGDnFillColor.Name = "FVG Down: Fill Color";
		Input_FVGDnFillColor.SetDescription("Fill Color Used for Rectangle");
		Input_FVGDnFillColor.SetColor(RGB(255, 128, 128));

		Input_FVGDnTransparencyLevel.Name = "FVG Down: Transparency Level";
		Input_FVGDnTransparencyLevel.SetDescription("Transparency Level for Rectangle Fill");
		Input_FVGDnTransparencyLevel.SetInt(65);
		Input_FVGDnTransparencyLevel.SetIntLimits(0, 100);

		Input_FVGDnDrawMidline.Name = "FVG Down: Draw Midline (Set Line Width to 1 or Higher)";
		Input_FVGDnDrawMidline.SetDescription("Draw Midline for FVG Rectangle. Requires Line Width of 1 or Higher.");
		Input_FVGDnDrawMidline.SetYesNo(0);

		Input_FVGDnExtendRight.Name = "FVG Down: Extend Right";
		Input_FVGDnExtendRight.SetDescription("Extend FVG Rectangle to Right of Chart Until Filled");
		Input_FVGDnExtendRight.SetYesNo(1);

		Input_FVGDnDeleteWhenFilled.Name = "FVG Down: Delete When Filled";
		Input_FVGDnDeleteWhenFilled.SetDescription("Hide Rectangle if Gap is Filled");
		Input_FVGDnDeleteWhenFilled.SetYesNo(1);

		Input_FVGDnMinGapSizeInTicks.Name = "FVG Down: Minimum Gap Size in Ticks";
		Input_FVGDnMinGapSizeInTicks.SetDescription("Only Process Gaps if greater or equal to Specified Gap Size");
		Input_FVGDnMinGapSizeInTicks.SetInt(1);
		Input_FVGDnMinGapSizeInTicks.SetIntLimits(1, INT_MAX);

		Input_FVGDnAllowCopyToOtherCharts.Name = "FVG Down: Allow Copy To Other Charts";
		Input_FVGDnAllowCopyToOtherCharts.SetDescription("Allow the FVG Rectangles to be Copied to Other Charts");
		Input_FVGDnAllowCopyToOtherCharts.SetYesNo(0);

		// General settings
		Input_FVGMaxBarLookback.Name = "Maximum Bar Lookback (0 = ALL)";
		Input_FVGMaxBarLookback.SetDescription("This Sets the Maximum Number of Bars to Process");
		Input_FVGMaxBarLookback.SetInt(200);
		Input_FVGMaxBarLookback.SetIntLimits(0, MAX_STUDY_LENGTH);

		return;
	}

	// See if we are capping max bars to check back
	if (Input_FVGMaxBarLookback.GetInt() == 0)
		sc.DataStartIndex = MIN_START_INDEX; // Need at least three bars to calculate
	else
		sc.DataStartIndex = sc.ArraySize - 1 - Input_FVGMaxBarLookback.GetInt() + MIN_START_INDEX;

	std::vector<FVGRectangle> FVGRectangles;
	std::vector<HLForBarIndex> HLForBarIndexes;
	int UniqueLineNumber = 8675309; // Jenny Jenny!

	// Min Gap Tick Size
	float FVGUpMinTickSize = float(Input_FVGUpMinGapSizeInTicks.GetInt()) * sc.TickSize;
	float FVGDnMinTickSize = float(Input_FVGDnMinGapSizeInTicks.GetInt()) * sc.TickSize;

	// Loop through bars and process FVG's
	// 1 is the current bar that is closed
	// 3 is the 3rd bar back from current bar
	for (int BarIndex = sc.DataStartIndex; BarIndex < sc.ArraySize - 1; BarIndex++)
	{
		// Store HL for each bar index. May need to revisit to see if better way
		HLForBarIndex TmpHLForBarIndex = {
			BarIndex,
			sc.High[BarIndex],
			sc.Low[BarIndex]
		};
		HLForBarIndexes.push_back(TmpHLForBarIndex);

		//
		// Reference logic borrowed from this indicator
		// https://www.tradingview.com/script/u8mKo7pb-muh-gap-FAIR-VALUE-GAP-FINDER/
		float L1 = sc.Low[BarIndex];
		float H1 = sc.High[BarIndex];
		float L3 = sc.Low[BarIndex - 2];
		float H3 = sc.High[BarIndex - 2];
		
		bool FVGUp = (H3 < L1) && (L1 - H3 >= FVGUpMinTickSize);
		bool FVGDn = (L3 > H1) && (L3 - H1 >= FVGDnMinTickSize);

		// Store the FVG Up
		if (FVGUp && Input_FVGUpEnabled.GetYesNo())
		{
			FVGRectangle TmpUpRect = {
				UniqueLineNumber + BarIndex, // Tool.LineNumber
				BarIndex - 2, // Tool.BeginIndex
				H3, // Tool.BeginValue
				BarIndex,// Tool.EndIndex
				L1, // Tool.EndValue
				false, // FVGEnded
				true // FVGUp
			};
			FVGRectangles.push_back(TmpUpRect);
		}

		// Store the FVG Dn
		if (FVGDn && Input_FVGDnEnabled.GetYesNo())
		{		
			FVGRectangle TmpDnRect = {
				UniqueLineNumber + BarIndex, // Tool.LineNumber
				BarIndex - 2, // Tool.BeginIndex
				L3, // Tool.BeginValue
				BarIndex,// Tool.EndIndex
				H1, // Tool.EndValue
				false, // FVGEnded
				false  // FVGUp
			};
			FVGRectangles.push_back(TmpDnRect);
		}
	}

	// Draw FVG Rectangles
	for (int i = 0; i < FVGRectangles.size(); i++)
	{
		s_UseTool Tool;
		Tool.Clear();
		Tool.ChartNumber = sc.ChartNumber;
		Tool.LineNumber = FVGRectangles.at(i).LineNumber;
		Tool.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;

		Tool.BeginIndex = FVGRectangles.at(i).ToolBeginIndex;
		Tool.BeginValue = FVGRectangles.at(i).ToolBeginValue;
		Tool.EndIndex = FVGRectangles.at(i).ToolEndIndex;
		Tool.EndValue = FVGRectangles.at(i).ToolEndValue;

		// Default to non-user drawing
		Tool.AddAsUserDrawnDrawing = 0;
		Tool.AllowCopyToOtherCharts = 0;

		if (FVGRectangles.at(i).FVGUp)
		{
			// FVG Up
			Tool.Color = Input_FVGUpLineColor.GetColor();
			Tool.SecondaryColor = Input_FVGUpFillColor.GetColor();
			Tool.LineWidth = Input_FVGUpLineWidth.GetInt();
			Tool.TransparencyLevel = Input_FVGUpTransparencyLevel.GetInt();
			Tool.AddMethod = UTAM_ADD_OR_ADJUST;

			if (Input_FVGUpDrawMidline.GetYesNo())
				Tool.DrawMidline = 1;

			// If we want to allow this to show up on other charts, need to set it to user drawing
			if (Input_FVGUpAllowCopyToOtherCharts.GetYesNo())
			{
				Tool.AddAsUserDrawnDrawing = 1;
				Tool.AllowCopyToOtherCharts = 1;
			}

			// In our High/Low Bar Index, check if we have an equal or lower Low than the FVG
			// If True, then this FVG will end at that index if extending Right
			auto it = find_if(
				HLForBarIndexes.begin(),
				HLForBarIndexes.end(),
				[=](HLForBarIndex& HLIndex)
				{
					return HLIndex.Index > FVGRectangles.at(i).ToolEndIndex && HLIndex.Low <= FVGRectangles.at(i).ToolBeginValue;
				}
			);
			// If true, we have an end index for this FVG and it hasn't yet been flagged as ended
			if (it != HLForBarIndexes.end() && !FVGRectangles.at(i).FVGEnded)
			{
				// FVG has ended, so then see if we want to show it or not...
				if (Input_FVGUpDeleteWhenFilled.GetYesNo())
					continue;

				// If here, we have an ending FVG that we want to show... Now need to see which EndIndex to use
				if (Input_FVGUpExtendRight.GetYesNo())
					Tool.EndIndex = it->Index; // Extending, so show it based on where we found the equal/lower bar low index

				// Flag this as ended now
				FVGRectangles.at(i).FVGEnded = true;
			}
			else
			{
				// If here, the FVG has not ended, so set to last closed bar if extending
				if (Input_FVGUpExtendRight.GetYesNo())
					Tool.EndIndex = sc.ArraySize - 1;
			}			
		}
		else
		{
			// FVG Down
			Tool.Color = Input_FVGDnLineColor.GetColor();
			Tool.SecondaryColor = Input_FVGDnFillColor.GetColor();
			Tool.LineWidth = Input_FVGDnLineWidth.GetInt();
			Tool.TransparencyLevel = Input_FVGDnTransparencyLevel.GetInt();
			Tool.AddMethod = UTAM_ADD_OR_ADJUST;

			if (Input_FVGDnDrawMidline.GetYesNo())
				Tool.DrawMidline = 1;

			// If we want to allow this to show up on other charts, need to set it to user drawing
			if (Input_FVGDnAllowCopyToOtherCharts.GetYesNo())
			{
				Tool.AddAsUserDrawnDrawing = 1;
				Tool.AllowCopyToOtherCharts = 1;
			}
			
			// In our High/Low Bar Index, check if we have an equal or higher High than the FVG
			// If True, then this FVG will end at that index if extending Right
			auto it = find_if(
				HLForBarIndexes.begin(),
				HLForBarIndexes.end(),
				[=](HLForBarIndex& HLIndex)
				{
					return HLIndex.Index > FVGRectangles.at(i).ToolEndIndex && HLIndex.High >= FVGRectangles.at(i).ToolBeginValue;
				}
			);
			// If true, we have an end index for this FVG and it hasn't yet been flagged as ended
			if (it != HLForBarIndexes.end() && !FVGRectangles.at(i).FVGEnded)
			{
				// FVG has ended, so then see if we want to show it or not...
				if (Input_FVGDnDeleteWhenFilled.GetYesNo())
					continue;

				// If here, we have an ending FVG that we want to show... Now need to see which EndIndex to use
				if (Input_FVGDnExtendRight.GetYesNo())
					Tool.EndIndex = it->Index; // Extending, so show it based on where we found the equal/lower bar low index

				// Flag this as ended now
				FVGRectangles.at(i).FVGEnded = true;
			}
			else
			{
				// If here, the FVG has not ended, so set to last closed bar if extending
				if (Input_FVGDnExtendRight.GetYesNo())
					Tool.EndIndex = sc.ArraySize - 1;
			}
		}
		sc.UseTool(Tool);
	}

	// This study is being removed from the chart or the chart is being closed
	if (sc.LastCallToFunction)
	{
		for (int i = 0; i < FVGRectangles.size(); i++)
		{
			if (Input_FVGUpAllowCopyToOtherCharts.GetYesNo() && FVGRectangles.at(i).FVGUp)
				sc.DeleteUserDrawnACSDrawing(sc.ChartNumber, FVGRectangles.at(i).LineNumber);
			if (Input_FVGDnAllowCopyToOtherCharts.GetYesNo() && !FVGRectangles.at(i).FVGUp)
				sc.DeleteUserDrawnACSDrawing(sc.ChartNumber, FVGRectangles.at(i).LineNumber);
		}
	}
}