#ifndef UICONFIG_H
#define UICONFIG_H

#include <raylib.h>

namespace ui {

struct WindowPreset {
  int width;
  int height;
  const char *label;
};

namespace Window {
constexpr int kInitialWidth = 700;
constexpr int kInitialHeight = 512;
constexpr int kTargetFps = 60;
constexpr int kSizePresetCount = 4;

static constexpr WindowPreset kSizePresets[kSizePresetCount] = {
    {700, 512, "700 x 512"},
    {900, 650, "900 x 650"},
    {1100, 780, "1100 x 780"},
    {1300, 920, "1300 x 920"}};
} // namespace Window

namespace Board {
constexpr int kSquaresPerSide = 8;
constexpr int kMaxIndex = kSquaresPerSide - 1;
constexpr float kSourceSize = 142.0f;
constexpr float kGridLeftPx = 7.0f;
constexpr float kGridTopPx = 7.0f;
constexpr float kGridRightPx = 7.0f;
constexpr float kGridBottomPx = 7.0f;

constexpr float kMinPanelWidth = 180.0f;
constexpr float kMinBoardSize = 240.0f;

constexpr float kUiBaseWidth = 512.0f;
constexpr float kUiScaleMin = 0.85f;
constexpr float kUiScaleMax = 1.35f;
} // namespace Board

namespace Piece {
constexpr float kDefaultSizeFactor = 0.70f;
constexpr float kBottomPadding = 4.0f;
constexpr float kCenterOffsetX = 0.0f;

constexpr float kBoardScaleLarge = 1.12f;
constexpr float kCapturedScaleLarge = 1.15f;
} // namespace Piece

namespace RightPanel {
constexpr Color kBackground = {26, 30, 40, 255};
constexpr Color kDivider = {70, 80, 100, 255};

constexpr float kTopSectionY = 8.0f;
constexpr float kTopBottomGap = 12.0f;
constexpr float kBottomSectionMargin = 8.0f;

constexpr float kTitleFont = 22.0f;
constexpr float kTitlePadX = 12.0f;
constexpr float kTitlePadY = 8.0f;

constexpr float kCapturedScaleMin = 0.95f;
constexpr float kCapturedScaleMax = 2.60f;
constexpr int kCompactColumnsMaxScreenWidth = 700;

constexpr float kIconSizeCompact = 32.0f;
constexpr float kIconSizeRegular = 36.0f;
constexpr float kIconGapCompact = 6.0f;
constexpr float kIconGapRegular = 7.0f;
constexpr float kGridInsetX = 10.0f;
constexpr float kGridInsetWidth = 20.0f;
constexpr float kCapturedY = 46.0f;

constexpr float kEmptyX = 14.0f;
constexpr float kEmptyFont = 32.0f;
constexpr int kEmptyHeight = 86;
constexpr int kSectionTopPadding = 52;
} // namespace RightPanel

namespace IconButtons {
constexpr float kSize = 44.0f;
constexpr float kGap = 12.0f;
constexpr float kStartY = 236.0f;
constexpr float kHoverBoost = 1.5f;
constexpr float kRoundness = 0.35f;
constexpr int kSegments = 10;
constexpr float kBorderWidth = 2.0f;
constexpr float kIconSize = 24.0f;
constexpr float kTooltipFont = 14.0f;
constexpr float kTooltipYOffset = 20.0f;

constexpr Color kButtonFill = {48, 58, 78, 255};
constexpr Color kButtonFillHover = {72, 86, 112, 255};
constexpr Color kButtonBorder = {90, 105, 130, 255};
constexpr Color kButtonBorderHover = {132, 152, 190, 255};
constexpr Color kTooltipText = {210, 220, 235, 255};

constexpr float kFallbackRotateLabelX = 14.0f;
constexpr float kFallbackRotateLabelY = 9.0f;
constexpr float kFallbackSettingsOuterRadius = 9.0f;
constexpr float kFallbackSettingsInnerRadius = 4.5f;

constexpr float kRestartRingInner = 8.0f;
constexpr float kRestartRingOuter = 12.0f;
constexpr float kRestartRingStartDeg = 35.0f;
constexpr float kRestartRingEndDeg = 320.0f;
constexpr int kRestartRingSegments = 24;

constexpr float kRestartArrowA_X = 9.0f;
constexpr float kRestartArrowA_Y = -12.0f;
constexpr float kRestartArrowB_X = 15.0f;
constexpr float kRestartArrowB_Y = -6.0f;
constexpr float kRestartArrowC_X = 6.0f;
constexpr float kRestartArrowC_Y = -4.0f;
} // namespace IconButtons

namespace ActionButton {
constexpr float kRoundness = 0.25f;
constexpr int kSegments = 8;
constexpr float kBorderWidth = 2.0f;

constexpr Color kPositiveFill = {47, 114, 74, 255};
constexpr Color kPositiveFillHover = {62, 141, 92, 255};
constexpr Color kPositiveBorder = {74, 154, 107, 255};
constexpr Color kPositiveBorderHover = {110, 199, 152, 255};

constexpr Color kNegativeFill = {112, 56, 56, 255};
constexpr Color kNegativeFillHover = {138, 69, 69, 255};
constexpr Color kNegativeBorder = {165, 93, 93, 255};
constexpr Color kNegativeBorderHover = {207, 124, 124, 255};

constexpr Color kYesText = {240, 245, 240, 255};
constexpr Color kNoText = {245, 240, 240, 255};
} // namespace ActionButton

namespace OptionButton {
constexpr float kRoundness = 0.22f;
constexpr int kSegments = 8;
constexpr float kBorderWidth = 2.0f;
constexpr Color kFill = {48, 58, 78, 255};
constexpr Color kFillHover = {72, 86, 112, 255};
constexpr Color kBorder = {90, 105, 130, 255};
constexpr Color kBorderHover = {132, 152, 190, 255};
} // namespace OptionButton

namespace Highlight {
constexpr Color kLegalMove = {0, 255, 0, 90};
constexpr Color kDragHover = {80, 180, 255, 95};
constexpr Color kSelection = {255, 255, 0, 255};
constexpr float kSelectionLine = 3.0f;

constexpr Color kInvalidFill = {220, 50, 50, 85};
constexpr Color kInvalidBorder = {255, 70, 70, 255};
constexpr float kInvalidLine = 3.0f;
} // namespace Highlight

namespace Overlay {
constexpr Color kBase = {0, 0, 0, 120};
constexpr Color kGameOver = {0, 0, 0, 130};
} // namespace Overlay

namespace Dialog {
constexpr float kScreenMargin = 40.0f;

constexpr float kRestartWidth = 320.0f;
constexpr float kRestartHeight = 150.0f;
constexpr float kRestartYesX = 34.0f;
constexpr float kRestartYesBottom = 52.0f;
constexpr float kRestartButtonWidth = 110.0f;
constexpr float kRestartButtonHeight = 34.0f;

constexpr float kWindowSizeWidth = 340.0f;
constexpr float kWindowSizeHeight = 270.0f;
constexpr float kWindowOptionHeight = 40.0f;
constexpr float kWindowOptionGap = 10.0f;
constexpr float kWindowOptionStartY = 54.0f;
constexpr float kWindowOptionPadX = 24.0f;
constexpr float kWindowCloseButtonSize = 28.0f;
constexpr float kWindowCloseMargin = 10.0f;

constexpr float kPromotionWidth = 320.0f;
constexpr float kPromotionHeight = 130.0f;
constexpr float kPromotionGap = 10.0f;
constexpr float kPromotionTotalPad = 36.0f;
constexpr float kPromotionStartX = 18.0f;
constexpr float kPromotionY = 52.0f;
constexpr int kPromotionOptionCount = 4;

constexpr float kRoundness = 0.16f;
constexpr int kRoundSegments = 12;
constexpr float kBorderWidth = 2.0f;
constexpr Color kBackground = {32, 38, 50, 255};
constexpr Color kBorder = {110, 124, 150, 255};
constexpr Color kTextPrimary = {235, 235, 235, 255};
constexpr Color kTextSecondary = {195, 205, 220, 255};

constexpr float kWindowSizeTitleY = 16.0f;
constexpr float kPromotionTitleY = 14.0f;

constexpr float kRestartTitleFont = 24.0f;
constexpr float kRestartBodyFont = 16.0f;
constexpr float kRestartActionFont = 20.0f;
constexpr float kRestartTitleY = 22.0f;
constexpr float kRestartBodyY = 58.0f;

constexpr float kWindowTitleFont = 22.0f;
constexpr float kWindowOptionFont = 18.0f;
constexpr float kWindowCloseFont = 20.0f;

constexpr float kPromotionTitleFont = 20.0f;
constexpr float kPromotionPieceScale = 1.10f;

constexpr Color kCloseText = {245, 240, 240, 255};
} // namespace Dialog

namespace GameOverDialog {
constexpr float kWidth = 360.0f;
constexpr float kHeight = 168.0f;
constexpr float kRoundness = 0.15f;
constexpr int kRoundSegments = 12;
constexpr float kBorderWidth = 2.0f;

constexpr float kTitleY = 24.0f;
constexpr float kBodyY = 78.0f;
constexpr float kHintY = 118.0f;

constexpr float kTitleFont = 30.0f;
constexpr float kBodyFont = 18.0f;
constexpr float kHintFont = 15.0f;

constexpr Color kBackground = {30, 35, 48, 255};
constexpr Color kBorder = {112, 125, 150, 255};
constexpr Color kBody = {232, 238, 246, 255};
constexpr Color kHint = {176, 188, 205, 255};
constexpr unsigned int kCheckmateTitleColor = 0xFFDCAAFF;
constexpr unsigned int kStalemateTitleColor = 0xDCEBFFFF;
} // namespace GameOverDialog

namespace BurnEffect {
constexpr int kFrameCount = 8;
constexpr double kFrameSpeed = 12.0;
constexpr int kSecondTierCaptureCount = 6;
constexpr int kStartCaptureCount = 2;
constexpr int kMaxEffectiveCaptureCount = 5;

constexpr float kSizeBase = 1.34f;
constexpr float kSizeLinear = 0.16f;
constexpr float kSizeQuadratic = 0.05f;
constexpr float kSizeMax = 2.45f;

constexpr float kLiftBase = 0.34f;
constexpr float kLiftLinear = 0.04f;
constexpr float kLiftFromSize = 0.22f;
constexpr float kLiftRefSize = 1.30f;
constexpr float kLiftMax = 0.64f;

constexpr unsigned char kTintAlpha = 195;
} // namespace BurnEffect

namespace CapturePopup {
constexpr int kStartCaptureCount = 2;
constexpr int kShakeStartCaptureCount = 3;
constexpr int kMaxEffectiveCaptureCount = 5;

constexpr float kIntensityPerCapture = 0.22f;
constexpr float kBurstEasePower = 1.35f;
constexpr float kScalePulseFactor = 0.52f;
constexpr float kFloatBase = 0.22f;
constexpr float kFloatPerIntensity = 0.04f;

constexpr float kBoxAlphaBase = 185.0f;
constexpr float kBoxAlphaPerIntensity = 25.0f;

constexpr float kShakeStrengthBase = 0.045f;
constexpr double kShakePhaseSpeed = 62.0;
constexpr float kShakePhaseYMultiplier = 1.27f;
constexpr float kShakeYStrengthMultiplier = 0.30f;

constexpr float kFontBase = 0.27f;
constexpr float kFontIntensityFactor = 0.02f;
constexpr int kMinFont = 14;

constexpr float kPadX = 0.11f;
constexpr float kPadY = 0.06f;
constexpr float kBadgeRightInset = 0.02f;
constexpr float kBadgeTopInset = 0.02f;

constexpr float kBadgeRoundness = 0.34f;
constexpr int kBadgeSegments = 8;
constexpr float kBadgeBorderWidth = 1.8f;

constexpr Color kBadgeBackground = {36, 40, 52, 255};
constexpr unsigned int kBorderColor = 0xFFD278FF;
constexpr unsigned int kTextColor = 0xFFF5BEFF;
constexpr unsigned int kOverheatBorderColor = 0xFF5F5FFF;
constexpr unsigned int kOverheatTextColor = 0xFF7878FF;
} // namespace CapturePopup

namespace Animation {
constexpr float kCastlingTweenDurationSeconds = 0.22f;
constexpr float kInvalidHighlightDurationSeconds = 0.55f;
constexpr float kCapturePopupDurationSeconds = 1.05f;
constexpr float kCapturePopupDurationPerExtraCapture = 0.08f;
constexpr float kCapturePopupDurationBaseFromMove = 1.00f;
} // namespace Animation

} // namespace ui

#endif // UICONFIG_H
