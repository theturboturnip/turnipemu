#pragma once

#include <cmath>

#include "turnipemu/types.h"
#include "turnipemu/utils.h"
#include "turnipemu/memory/map.h"

namespace TurnipEmu::GBA{
	class SoundEngine : public Memory::RangeController {
	public:
		SoundEngine();

		const std::vector<uint8_t>& generateSamples(size_t count);
		
		// TODO: Define sound behaviour

		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;
		
	private:
		std::vector<uint8_t> samples;

#pragma pack(1)
		struct DutyLengthEnvelopeRegister {
			uint8_t soundLength : 6; // Bits 0-5. Tone length in seconds is calculated using (64-n)/256
			// Bits 6-7. 0=1/8, 1=1/4, 2=1/2, 3=3/4.
			// For a value of 25% the tone is on 25% of the time: -___-___, for 50%: --__--__ etc.
			uint8_t soundDuty : 2;
			uint8_t envelopeStepTime : 3; // Bits 8-10. Every N/64 seconds the envelope will increase/decrease.
			bool envelopeDirection_bool : 1; // Bit 11.
			inline int envelopeDirection(){
				return envelopeDirection_bool ? 1 : -1;
			}
			uint8_t envelopeInitialVolume : 4; // Bits 12-15.

			//constexpr static byte ReadMasks[] = {0b110000000, byte(-1)};
		};
		static_assert(sizeof(DutyLengthEnvelopeRegister) == 2);
		struct FrequencyControlRegister {
			TURNIPEMU_SPLIT_HALFWORD_OR_LESS(frequency, 11); // Bits 0-10. Actual Frequency calculated using 131072/(2048-N)
			uint8_t dummy : 3; // Bits 11-13
			bool useLength : 1; // Bit 14. If true, sound output is halted when the length expires.
			bool restart : 1; // Bit 15. When set to true, the sound is restarted(?)
			
			//constexpr static byte ReadMasks[] = {0, byte(1 << 6)};
		};
		static_assert(sizeof(FrequencyControlRegister) == 2);
		
		union Channel1 {
			struct SweepShiftRegister {
				uint8_t power : 3; // Bits 0-2, F after t shifts = F(t) = F(t-1) +/- F(t-1)/(2^N)
				bool direction_bool : 1; // Bit 3
				inline int direction() {
					return direction_bool ? -1 : 1;
				}
				uint8_t time : 3; // Bits 4-6. Time between shifts in increments of 7.8ms. Setting to 0 disables sweepShift.
				uint16_t dummy : 9; // Bits 7-15.
			};
			static_assert(sizeof(SweepShiftRegister) == 2);

			byte data[8];
			struct {
				SweepShiftRegister sweepShift;
				DutyLengthEnvelopeRegister dutyLengthEnvelope;
				FrequencyControlRegister frequencyControl;
				uint16_t dummy;
			};
		} channel1;

		union Channel2 {
			byte data[8];
			struct {
				DutyLengthEnvelopeRegister dutyLengthEnvelope;
				uint16_t dummy;
				FrequencyControlRegister frequencyControl;
				uint16_t dummy1;
			};
		} channel2;

		// Channel 3 has two RAM Banks for filling with wave data.
		union Channel3 {
			struct RAMSelectRegister {
				uint8_t dummy : 5; // Bits 0-4
				bool waveRamDimensions : 1; // Bit 5. 0 = One Bank, 1 = Two Banks
				uint8_t selectedBankIndex : 1; // Bit 6. When using One Bank, this bank will be played back, and writing to the RAM bank area will write to the other bank. When using Two Banks, this bank will be played back first, and (?)writing to the RAM bank area is prohibited(?)
				bool enablePlayback : 1; // Bit 7
				uint8_t dummy1;
			};
			static_assert(sizeof(RAMSelectRegister) == 2);
			struct LengthVolumeRegister {
				uint8_t length : 8; // Bits 0-7. Sound Length in seconds is calculated with (256-N)/255
				uint8_t dummy : 5; // Bits 8-12
				uint8_t soundVolume_notForced : 2; // Bits 13-14.
				bool soundVolume_isForced : 1; // Bit 15
				inline uint8_t soundVolumeTimes4(){
					if (soundVolume_isForced){
						return 3; // 75% 
					}
					switch (soundVolume_notForced){
					case 0:
						return 0; // 0%
					case 1:
						return 4; // 100%
					case 2:
						return 2; // 50%
					case 3:
						return 1; // 25%
					default:
						assert(false);
						return 0;
					}
				}
			};
			static_assert(sizeof(LengthVolumeRegister) == 2);
			
			byte data[8];
			struct {
				RAMSelectRegister ramSelect;
				LengthVolumeRegister lengthVolume;
				FrequencyControlRegister frequencyControl;
				uint16_t dummy1;
			};
		} channel3;

		union Channel4 {
			struct NoiseFrequencyControlRegister {
				uint8_t frequencyDividingRatio_enum : 3; // Bits 0-2
				inline float dividingRatio() {
					if (frequencyDividingRatio_enum == 0) return 0.5f;
					else return static_cast<float>(frequencyDividingRatio_enum);
				}
				bool counterWidth_enum : 1; // Bit 3. False=15 Bit shift value starting at 0x4000, True=7 Bit shift Value starting at 0x40
				uint8_t frequencyPower : 4; // Bits 4-7.
				inline float shiftFrequency() {
					return std::pow(2, 19 - (frequencyPower + 1)) / dividingRatio();
				}

				int : 6; // Bits 8-13
				bool useLength : 1; // Bit 14. If true, sound output is halted when the length expires.
				bool restart : 1; // Bit 15. When set to true, the sound is restarted(?)
					
				// Generating Noise
				// Noise randomly switches between HIGH and LOW levels, the output levels are calculated by a shift register (X), at the selected frequency, as such:
				// 7bit:  X=X SHR 1, IF carry THEN Out=HIGH, X=X XOR 60h ELSE Out=LOW
				// 15bit: X=X SHR 1, IF carry THEN Out=HIGH, X=X XOR 6000h ELSE Out=LOW
				// The initial value when (re-)starting the sound is X=40h (7bit) or X=4000h (15bit). The data stream repeats after 7Fh (7bit) or 7FFFh (15bit) steps.
			};
			static_assert(sizeof(NoiseFrequencyControlRegister) == 2);
			
			byte data[8];
			struct {
				DutyLengthEnvelopeRegister lengthEnvelope; // Duty not used.
				uint16_t dummy;
				NoiseFrequencyControlRegister noiseFrequency;
				uint16_t dummy1;
			};
		} channel4;

		union {
			struct SoundBias {
				int : 1;
				TURNIPEMU_SPLIT_HALFWORD_OR_LESS(bias, 9)
				int : 4;
				uint8_t resampleEnum : 2; // Don't bother trying to figure this out for now, it's really weird
			};
			static_assert(sizeof(SoundBias) == 2);
			
			byte data[12];
			struct {
				// halfword
				uint8_t volumeRight : 3;
				int : 1;
				uint8_t volumeLeft : 3;
				int : 1;
				uint8_t channelEnableBitsRight : 4;
				uint8_t channelEnableBitsLeft : 4;

				// halfword
				uint8_t masterVolume : 2; // 0 = 25%, 1 = 50%, 2 = 100%, 3 = Prohibited
				bool dmaSoundAFullVolume : 1; // When false, volume is 50%
				bool dmaSoundBFullVolume : 1; // See above
				int : 4;
				bool dmaSoundAEnableRight : 1;
				bool dmaSoundAEnableLeft : 1;
				uint8_t dmaSoundATimerSelect : 1;
				bool dmaSoundAReset : 1;
				bool dmaSoundBEnableRight : 1;
				bool dmaSoundBEnableLeft : 1;
				uint8_t dmaSoundBTimerSelect : 1;
				bool dmaSoundBReset : 1;

				// halfword
				bool sound1IsOn : 1;
				bool sound2IsOn : 1;
				bool sound3IsOn : 1;
				bool sound4IsOn : 1;
				int : 4;
				bool psgFifoEnabled : 1;
				int : 7;

				// halfword
				int : 16;

				// halfword
				SoundBias biasControl;
				
				// halfword
				int : 16;
			};
		} control;

		// TODO: Sound registers from 0x0'0400'0090 onwards. Channel 3 wave RAM, Channel A/B FIFOs
		
#pragma pack()
	};
}
