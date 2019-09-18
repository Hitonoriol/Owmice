#define BYTES_PER_SECTOR 512

struct ATA_INTERFACE {
	uint8_t master;
	uint16_t dataPort;
	uint16_t errorPort;
	uint16_t sectorCountPort;
	uint16_t lbaLowPort;
	uint16_t lbaMidPort;
	uint16_t lbaHiPort;
	uint16_t devicePort;
	uint16_t commandPort;
	uint16_t controlPort;
};

struct ATA_INTERFACE* newATA(uint8_t master, uint16_t portBase) {
	struct ATA_INTERFACE* ret = (struct ATA_INTERFACE*)malloc(sizeof(struct ATA_INTERFACE));
	ret->master = master;
	ret->dataPort = portBase;
	ret->errorPort = portBase + 0x1;
	ret->sectorCountPort = portBase + 0x2;
	ret->lbaLowPort = portBase + 0x3;
	ret->lbaMidPort = portBase + 0x4;
	ret->lbaHiPort = portBase + 0x5;
	ret->devicePort = portBase + 0x6;
	ret->commandPort = portBase + 0x7;
	ret->controlPort = portBase + 0x206;
	return ret;
}

uint8_t ATA_identify(struct ATA_INTERFACE* iface) {
	write_port(iface->devicePort, iface->master ? 0xA0 : 0xB0);
	write_port(iface->controlPort, 0);

	write_port(iface->devicePort, 0xA0);
	uint8_t status = read_port(iface->commandPort);
	if(status == 0xFF) return 1;

	write_port(iface->devicePort, iface->master ? 0xA0 : 0xB0);
	write_port(iface->sectorCountPort, 0);
	write_port(iface->lbaLowPort, 0);
	write_port(iface->lbaMidPort, 0);
	write_port(iface->lbaHiPort, 0);
	write_port(iface->commandPort, 0xEC); // Identify command

	status = read_port(iface->commandPort);
	if(!status) return 1;

	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
		status = read_port(iface->commandPort);
	}

	if(status & 0x01) return 1;

	for(int i=0; i<256; i++) read_port16(iface->dataPort);
	return 0;
}

uint8_t* ATA_read28(struct ATA_INTERFACE* iface, uint32_t sector) {
	if(sector > 0x0FFFFFFF) return 0;

	write_port(iface->devicePort, (iface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Wait for drive select to finish.
	uint8_t status;
	for(int i=0; i<5; i++) status = read_port(iface->commandPort);
	if(status == 0xFF) return 0;

	write_port(iface->errorPort, 0);
	write_port(iface->sectorCountPort, 1);
	write_port(iface->lbaLowPort, sector & 0x000000FF);
	write_port(iface->lbaMidPort, (sector & 0x0000FF00) >> 8);
	write_port(iface->lbaHiPort, (sector & 0x00FF0000) >> 16);
	write_port(iface->commandPort, 0x20);	// Read command.

	status = read_port(iface->commandPort);
	while((status & 0x80) && !(status & 0x01)) status = read_port(iface->commandPort);

	uint8_t* ret = (uint8_t*)malloc(BYTES_PER_SECTOR);
	for(int i=0; i<BYTES_PER_SECTOR; i+=2) {
		uint16_t data = read_port16(iface->dataPort);
		ret[i] = data & 0xFF;
		ret[i+1] = (data >> 8) & 0xFF;
	}

	return ret;
}

uint8_t ATA_write28(struct ATA_INTERFACE* iface, uint32_t sector, uint8_t* contents) {
	if(sector > 0x0FFFFFFF) return 1;
	cli();

	// WRITE
	write_port(iface->devicePort, (iface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Check that drive select worked.
	uint8_t status;
	for(int i=0; i<5; i++) status = read_port(iface->commandPort);
	if(status == 0xFF) return 1;

	write_port(iface->errorPort, 0);
	write_port(iface->sectorCountPort, 1);
	write_port(iface->lbaLowPort, sector & 0x000000FF);
	write_port(iface->lbaMidPort, (sector & 0x0000FF00) >> 8);
	write_port(iface->lbaHiPort, (sector & 0x00FF0000) >> 16);
	write_port(iface->commandPort, 0x30);	// Write command.

	// Wait until BSY=0 and DRQ=1.
	while((status & 0x80) || !(status & 0x08)) status = read_port(iface->commandPort);

	// Check ERR and DF are not set.
	if(status & (0x01 || 0x20)) return 2;

	// Start sending the data.
	for(int i=0; i<BYTES_PER_SECTOR; i+=2) {
		uint16_t data = contents[i];
		data |= ((uint16_t)contents[i+1]) << 8;
		port_write16(iface->dataPort, data);
	}

	// FLUSH
	// No need to drive select again.
	write_port(iface->commandPort, 0xE7);	// Flush command.

	for(int i=0; i<5; i++) status = read_port(iface->commandPort);
	if(!status) return 3;

	while((status & 0x80) && !(status & 0x01)) {
		status = read_port(iface->commandPort);
	}
	sti();

	return 0;
}

struct ATA_INTERFACE* master;
