#pragma once

#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

namespace TurnipEmu::GBA{
	class DMAEngine : public Memory::RangeController {
	public:
		DMAEngine();
		
		void execute(Memory::Map& memoryMap);
		bool canExecute();

		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;

	protected:
#pragma pack(1) // No Padding
		enum class AddressControl : uint8_t {
			Increment = 0,
				Decrement = 1,
				Fixed = 2,
				IncrementAndReload = 3 // Only allowed for destination, means that on a reload the destination for DMA is the same. Otherwise, the destination will increment forever
				};
		enum class StartTiming : uint8_t {
			Immediate = 0,
				VBlank = 1,
				HBlank = 2,
				Special = 3
				};
		enum class TransferType : bool {
			HalfWord = false,
				Word = true
				};
		class DMAChannel {
		public:
			union ExternalState {
				byte data[12];
				struct {
					word sourceAddressOnStart : 32;
					word destinationAddressOnStartOrReload : 32;
					halfword transferCountOnStart : 16;
					uint8_t padding : 5; // Bits 0-4
					AddressControl destinationAddressControl : 2; // Bits 5-6
					AddressControl sourceAddressControl : 2; // Bits 7-8, IncrementAndReload is prohibited
					bool repeat : 1; // Bit 9, Must be 0 if Game Pak DRQ bit set
					TransferType transferType : 1; // Bit 10
					bool gamePakDRQ : 1; // Bit 11. DMA 3 only, if set then transfer is controlled by the cartridge. Probably not supported.
					StartTiming startTiming : 2; // Bits 12-13, the meaning of Special depends on the channel: DMA 0=Prohibited, DMA 1/2=Sound FIFO, DMA3=Video Capture (External Hardware required).
					bool fireInterruptWhenDone : 1; // Bit 14
					bool enable : 1; // Bit 15
				};
			} externalState;
			static_assert(sizeof(ExternalState) == 12);
			word sourceAddress;
			word destinationAddress;
			halfword transferCount;

			inline bool enabled(){
				return externalState.enable;
			}
		};
		std::array<DMAChannel, 4> channels;
#pragma pack()
	};
}
