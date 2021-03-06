#pragma once

#include <cstddef>

#include "turnipemu/types.h"
#include "turnipemu/utils.h"
#include "turnipemu/memory/map.h"

namespace TurnipEmu::GBA{
	class LCDEngine : public Memory::Controller{
	public:
		// TODO: Everything else
		
		bool ownsAddress(uint32_t address) const override;
		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;
		// TODO: Memory cycles

	protected:
		byte* ownedAddressToByte(uint32_t address);
		
#pragma pack(1) // No Padding

		union Control{
			byte data[4];
			struct {
				uint8_t bgMode : 3; // Bits 0-2
				bool inCgbMode : 1; // Bit 3, should always be false
				uint8_t displayFrameSelect : 1; // Bit 4
				bool allowOAMAccessDuringHBlank : 1; // Bit 5
				uint8_t objCharacterVramMapping : 1; // Bit 6, 0 for 2D, 1 for 1D
				bool forceBlank : 1; // Bit 7
					
				bool displayBg0 : 1; // Bit 8
				bool displayBg1 : 1; // Bit 9
				bool displayBg2 : 1; // Bit 10
				bool displayBg3 : 1; // Bit 11
				bool displayObjects : 1; // Bit 12
				bool displayWindow0 : 1; // Bit 13
				bool displayWindow1 : 1; // Bit 14
				bool objWindowDisplayFlag : 1; // Bit 15 (Not sure what this does?)

				bool greenSwap : 1; // Bit 16, Undocumented, when displaying two adjacent pixels swap the green components
				uint16_t unused : 15;
			};
		};
		static_assert(sizeof(Control) == 4, "LCDEngine::Control must be a full word");

		union Status {
			byte data[4];
			struct {
				bool vblankFlag : 1; // Bit 0 (Read only, use as internal state?)
				bool hblankFlag : 1; // Bit 1 (Read only, use as internal state?)
				bool vCountIsEqualToCurrentScanline : 1; // Bit 2 (Read only, use as internal state)
				bool enableVblankInterrupt : 1; // Bit 3 (R/W)
				bool enableHblankInterrupt : 1; // Bit 4 (R/W)
				bool enableVCounterInterrupt : 1; // Bit 5 (R/W)
				bool dsiLcdInited : 1; // Bit 6 (Read only, set on read, only used on DSi)
				uint8_t ndsMSBOfVCountDelta : 1; // Bit 7 (Read only, set on read, only used on DS)
					
				uint8_t vCount : 8; // Bits 8-15, (R/W)
					
				uint8_t currentScanline : 8; // Bits 16-23 (Read only, use as internal state)
					
				uint8_t ndsMSBOfCurrentScanline : 1; // Bit 24 (Read only, set on read, only used on DS)
				uint8_t unused : 7;
			};
		};
		static_assert(sizeof(Status) == 4, "LCDEngine::Status must be a full word");
		constexpr static byte statusFirstByteWriteMask = 0b00111000;

		union BGControl {
			byte data[2];
			struct {
				uint8_t priority : 2; // Bits 0-1, 0 = Highest Priority
				uint8_t characterStartBlock : 2; // Bits 2-3, units of 16KB, basically BG Tile Data
				uint8_t unused : 2; // Bits 4-5, should be 0
				bool enableMosaic : 1; // Bit 6
				uint8_t colorSpace : 1; // Bit 7, 0 for 16 colors and 16 palettes, 1 for 256 colors and 1 palette
					
				uint8_t screenStartBlock : 5; // Bits 8-12, units of 2KB, basically BG Map Data
				bool wrapMode : 1; // Bit 13, 0 for clamp, 1 for wrap, used for BGs 2 and 3 ONLY
				uint8_t screenSize : 2; // Bit 14-15, different meanings depending on the BG mode
				// In Text Mode, there are N areas of 32x32 tiles.
				// These areas read their map data from adjacent spaces
				// i.e. area 0 reads starting at block no. screenStartBlock, area 1 reads starting at block no. screenStartBlock + 1
				// Screen Size | Areas
				//      0      | 1 area
				//      1      | 2 areas, R = 0, L = 1
				//      2      | 2 areas, T = 0, B = 1 (?)
				//      3      | 4 areas, TL = 0, TR = 1, BL = 2, BR = 3
				// The screen will always wrap in Text Mode.

				// In Rotation/Scale modem there is 1 area of varying size
				// Screen Size | Size
				//      0      | 128x128
				//      1      | 256x256
				//      2      | 512x512
				//      3      | 1024x1024
				// Rotation/Scale mode respects wrapMode
			};
		};
		static_assert(sizeof(BGControl) == 2, "LCDEngine::BGControl must be a half word");
		// Only used in Text Mode
		struct BGOffset {
			uint8_t x_high : 8;
			uint8_t x_low : 1;
			uint8_t unused : 7;
				
			uint8_t y_high : 8;
			uint8_t y_low : 1;
			uint8_t unused1 : 7;
			
			uint16_t x() const {
				return (x_high << 1) + x_low;
			}
			uint16_t y() const {
				return (y_high << 1) + y_low;
			}
		};
		static_assert(sizeof(BGOffset) == 4, "LCDEngine::BGOffset must be a full word");

		struct BGFullFloat {
			uint8_t fractionalPortion : 8; // Bits 0-7
			uint8_t integerPortion_highest : 8; // Bits 8-15
			uint8_t integerPortion_high : 8; // Bits 16-23
			uint8_t integerPortion_low : 3; // Bits 24-26
			int sign : 1; // Bit 27, int : 1 is automatically the sign bit
			uint8_t unused : 4; // Bits 28-31

			operator float(){
				uint32_t integerPortion = (integerPortion_highest << 11) + (integerPortion_high << 3) + integerPortion_low;
				return sign * (integerPortion + (fractionalPortion / 256.0f)); // TODO: Are the brackets in the right place?
			}
		};
		static_assert(sizeof(BGFullFloat) == 4, "LCDEngine::BGFullFloat must be a full word");
		struct BGDeltaFloat {
			uint8_t fractionalPortion : 8; // Bits 0-7				
			uint8_t integerPortion : 7; // Bits 8-14
			int sign : 1; // Bit 15, int : 1 is automatically the sign bit

			operator float(){
				return sign * (integerPortion + (fractionalPortion / 256.0f)); // TODO: Are the brackets in the right place?
			}
		};
		static_assert(sizeof(BGDeltaFloat) == 2, "LCDEngine::BGDeltaFloat must be a half word");
		struct BGRotationScaleControl {
			BGDeltaFloat dx;
			BGDeltaFloat dmx;
			BGDeltaFloat dy;
			BGDeltaFloat dmy;

			BGFullFloat centerX;
			BGFullFloat centerY;
		};
		static_assert(sizeof(BGRotationScaleControl) == 16, "LCDEngine::BGRotationScaleControl must be 4 words");

		// From 0x4000008 to 0x4000040
		union BGData {
			byte data[0x40 - 0x08]; // 56 bytes, 14 words
			struct {
				// 0x08 - 0x0F
				BGControl bg0Control;
				BGControl bg1Control;
				BGControl bg2Control;
				BGControl bg3Control;

				// 0x10 - 0x1F
				BGOffset bg0Offset;
				BGOffset bg1Offset;
				BGOffset bg2Offset;
				BGOffset bg3Offset;

				// 0x20 - 0x3F
				BGRotationScaleControl bg2RotScale;
				BGRotationScaleControl bg3RotScale;
			};
		};
		static_assert(sizeof(BGData) == (0x40 - 0x08), "LCDEngine::BGData must be the right size");

		struct WindowDimension {
			uint8_t maxValuePlus1; // If this is < minValue, it is interpreted as the maximum possible value of that dimension
			uint8_t minValue;
		};
		static_assert(sizeof(WindowDimension) == 2, "LCDEngine::WindowDimension must be a half word");

		struct EffectControl {
			bool bg0Enable : 1; // Bit 0
			bool bg1Enable : 1; // Bit 1
			bool bg2Enable : 1; // Bit 2
			bool bg3Enable : 1; // Bit 3
			bool objEnable : 1; // Bit 4
			bool colorEffectEnable : 1; // Bit 5
			uint8_t unused : 2; // Bits 6-7
		};
		static_assert(sizeof(EffectControl) == 1, "LCDEngine::EffectControl must be a single byte");

		struct MosaicControl {
			uint8_t bgMosaicHorizontalSizeMinus1 : 4; // Bits 0-3
			uint8_t bgMosaicVerticalSizeMinus1 : 4; // Bits 4-7
				
			uint8_t objMosaicHorizontalSizeMinus1 : 4; // Bits 8-11
			uint8_t objMosaicVerticalSizeMinus1 : 4; // Bits 12-15
				
			uint16_t unused;
		};
		static_assert(sizeof(MosaicControl) == 4, "LCDEngine::EffectControl must be a full word");

		struct ColorEffectData {
			bool allowBg0AsFirstTarget : 1;
			bool allowBg1AsFirstTarget : 1;
			bool allowBg2AsFirstTarget : 1;
			bool allowBg3AsFirstTarget : 1;
			bool allowTopmostObjAsFirstTarget : 1;
			bool allowBackdropAsFirstTarget : 1;
			uint8_t selectedEffect : 2;
				
			bool allowBg0AsSecondTarget : 1;
			bool allowBg1AsSecondTarget : 1;
			bool allowBg2AsSecondTarget : 1;
			bool allowBg3AsSecondTarget : 1;
			bool allowTopmostObjAsSecondTarget : 1;
			bool allowBackdropAsSecondTarget : 1;
			uint8_t unused : 2;

			uint8_t firstTargetCoefficient : 5; // Divided by 16 with the result capped at 1
			uint8_t unused1 : 3;
			uint8_t secondTargetCoefficient : 5; // Divided by 16 with the result capped at 1
			uint8_t unused2 : 3;
			
			uint8_t brightnessCoefficient : 5; // Divided by 16 with the result capped at 1
			uint32_t unused3 : 27;
		};
		static_assert(sizeof(ColorEffectData) == 8, "LCDEngine::ColorEffectCoefficients must be two words");
		
		union EffectData {
			byte data[0x18];
			struct {
				// 0x40 - 0x47
				WindowDimension window0HorizDimen;
				WindowDimension window1HorizDimen;
				WindowDimension window0VertDimen;
				WindowDimension window1VertDimen;
					
				// 0x48 - 0x4B
				EffectControl window0Effects;
				EffectControl window1Effects;
				EffectControl outsideWindowEffects;
				// Used on objects with the OBJ mode set to "OBJ Window". These objects aren't displayed normally
				EffectControl insideObjectWindowEffects;
				
				// 0x4C
				MosaicControl mosaicControl;

				// 0x50
				ColorEffectData colorEffectData;
			};
		};
		static_assert(sizeof(EffectData) == 0x18, "LCDEngine::EffectData must have the right size");

		struct Tile4 {
			byte data[32];
			inline uint8_t paletteIndex(uint8_t x, uint8_t y){
				return (data[4 * y + (x / 2)] >> (x % 2 == 0 ? 0 : 4)) & 0x0F;
			}
		};
		static_assert(sizeof(Tile4) == 32, "LCDEngine::Tile4 must be 32 bytes!");
		struct Tile8 {
			byte data[64];
			inline uint8_t paletteIndex(uint8_t x, uint8_t y){
				return data[y * 8 + x];
			}
		};
		static_assert(sizeof(Tile8) == 64, "LCDEngine::Tile8 must be 64 bytes!");
		struct TextMapItem {
			uint8_t tileNumber_high;
			uint8_t tileNumber_low : 2;

			bool horizontalFlip : 1;
			bool verticalFlip : 1;
			uint8_t paletteNumber : 4;
			
			inline uint16_t tileNumber(){
				return (tileNumber_high << 2) + tileNumber_low;
			}
		};
		static_assert(sizeof(TextMapItem) == 2, "LCDEngine::TextMapItem must be 2 bytes"); 

		struct ObjAttribute {
			uint8_t yCoordinate : 8;

			bool rotationScaling : 1;
			bool doubleSizeOrDisable : 1; // If rotatingAndScaling then this is doubleSize, otherwise this is disable
			uint8_t objMode : 2; // 0 = Normal, 1 = Semitransparent, 2 = OBJ Window, 3 = Prohibited
			bool objMosaic : 1;
			bool colorPaletteRatio : 1; // True for 256 color single palette, False for 16 colors/16 palettes
			uint8_t objShape : 2; // 0 = Square, 1 = Horizontal Rectangle, 2 = Vertical Rectangle, 3 = Prohibited

			uint8_t xCoordinate_high : 8;
			
			// Pretty sure unions/structs have to be byte-aligned and stuff, so repeat some information
			union {
				struct {
					uint8_t xCoordinate_low : 1;
					// Only when rotationScaling
					uint8_t rotationScalingParameter : 5;
					// 0 = 8x8 Square/16x8 Rectangle, 1 = 16x16 Sq/32x8 R, 2 = 32x32 Sq/32x16 R, 3 = 64x64 Sq/64x32 R
					// (The rectangle coords switch depending on objShape)
					uint8_t objSize : 2; 
				};
				struct {
					uint8_t dummy1 : 4;
					// When not rotationScaling
					bool flipHorizontal : 1;
					bool flipVertical : 1;
					uint8_t dummy2 : 2;
				};
			};

			uint8_t tileNumber_high;

			uint8_t tileNumber_low : 2;
			uint8_t relativePriority : 2;
			uint8_t paletteNumber : 4;
			
			uint16_t dummy3; // This is used as parts of the rotation/scaling parameters

			inline uint16_t xCoordinate(){
				return (xCoordinate_high << 1) + xCoordinate_low;
			}
			inline uint16_t tileNumber(){
				return (tileNumber_high << 2) + tileNumber_low;
			}
		};
		static_assert(sizeof(ObjAttribute) == 8);

		union IORegisters {
			byte data[0x60];
			struct {
				Control control; // 0x00 - 0x03
				Status status; // 0x04 - 0x07
				BGData bgData; // 0x08 - 0x3F
				EffectData effectData; // 0x40 - 0x58
			};
		} io; // 0x0400'0000 - 0x0400'0060
		static_assert(sizeof(IORegisters) == 0x60);

		union PaletteRam {
			byte data[0x400];
		} paletteRam; // 0x0500'0000 - 0x0500'0400
		
		union Vram {
			byte data[0x18000];
			struct {
				union {
					Tile4 dataAs4BitBGTiles[0x10000/sizeof(Tile4)];
					Tile8 dataAs8BitBGTiles[0x10000/sizeof(Tile8)];
					
					TextMapItem dataAsTextMapItems[0x10000/sizeof(TextMapItem)];
					byte dataAsRotScaleMap[0x10000];
				};
				Tile8 objectTiles[0x8000/sizeof(Tile8)];
			} tileModes; // Modes 0, 1, 2
			struct {
				byte frame0Buffer[0x14000];
				Tile8 objectTiles[0x4000/sizeof(Tile8)];
			} singleBufferedBitmapMode; // Mode 3
			struct {
				byte frame0Buffer[0xA000];
				byte frame1Buffer[0xA000];
				Tile8 objectTiles[0x4000/sizeof(Tile8)];
			} doubleBufferedBitmapModes; // Modes 4, 5
		} vram; // 0x0600'0000 - 0x0601'7FFF
		static_assert(sizeof(Vram) == 0x18000, "VRam must take exactly 0x18000 bytes");

		union ObjAttributes {
			byte data[0x400];
			ObjAttribute attributeData[0x400 / sizeof(ObjAttribute)];
		} objectAttributes; // 0x0700'0000 - 0x0700'0400
		static_assert(sizeof(ObjAttributes) == 0x400);
#pragma pack()
	};
}
