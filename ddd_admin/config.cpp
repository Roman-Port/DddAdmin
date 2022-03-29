#include "config.h"
#include <stdio.h>

bool ddd_read_line(FILE* file, char* output, size_t max_output_size) {
	char temp;
	for (int i = 0; i < max_output_size - 1; i++) {
		if (fread(&temp, 1, 1, file) != 1)
			return false;
		if (temp == '\r') {
			continue;
		}
		else if (temp == '\n') {
			output[i] = 0;
			return true;
		}
		else {
			output[i] = temp;
		}
	}
	return false;
}

bool ddd_load_admin_config(ISmmAPI* plugin, ddd_admin_config_t* result)
{
	//Create filename
	char filename[256];
	plugin->PathFormat(filename, sizeof(filename), "%s/cfg/ddd_admin_config.txt", plugin->GetBaseDir());

	//Open
	printf("ddd_admin:INFO // Attempting to load config file from \"%s\"...", filename);
	FILE* file = fopen(filename, "rb");
	if (file == 0) {
		printf("FAILED TO OPEN\n");
		return false;
	}

	//Read lines
	if (ddd_read_line(file, result->server_ip, sizeof(result->server_ip)) && ddd_read_line(file, result->server_port, sizeof(result->server_port)) && ddd_read_line(file, result->server_psk, sizeof(result->server_psk))) {
		printf("OK\n");
		return true;
	}
	else {
		printf("MALFORMATTED\n");
		return false;
	}
}
