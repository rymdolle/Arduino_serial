#include <libserialport.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>

#include "serial/serial_protocol.h"

std::string read_string(struct sp_port *port)
{
  std::string text;
  // Read one byte at a time until null terminator
  for (char byte; sp_blocking_read(port, &byte, 1, 0);) {
    if (0 == byte)
      break;
    text.push_back(byte);
  }
  return text;
}

int main(int argc, char *argv[])
{
  struct sp_port **ports;
  sp_return error = sp_list_ports(&ports);
  if (error != SP_OK) {
    std::cerr << "List ports failed: " << sp_last_error_message() << '\n';
    exit(1);
  }

  // List available ports
  int ports_len;
  std::cout << "Available ports:\n";
  for (ports_len = 0; ports[ports_len] != NULL; ++ports_len) {
    char *port_name = sp_get_port_name(ports[ports_len]);
    std::cout << std::setw(2) << ports_len + 1 << ". " << port_name << '\n';
  }

  if (ports_len == 0) {
    std::cout << "No available ports\n";
    exit(0);
  }

  // Select port to use
  struct sp_port *port = nullptr;
  do {
    std::string text;
    std::cout << '\n'
              << "Select port or 0 to exit: ";
    std::getline(std::cin, text);
    int input = std::atoi(text.c_str());
    if (input > 0 && input <= ports_len) {
      // Copy selected port
      error = sp_copy_port(ports[input-1], &port);
      if (error != SP_OK) {
        std::cerr << "Could not copy port.\n"
                  << sp_last_error_message() << '\n';
        exit(1);
      }
    } else if (input == 0) {
      std::cout << "Exit\n";
      exit(0);
    } else {
      std::cerr << "Invalid port number.\n";
    }
  } while (port == nullptr);

  // Free port list
  sp_free_port_list(ports);

  std::cout << "Connecting to " << sp_get_port_name(port);

  // Open for read/write
  error = sp_open(port, SP_MODE_READ_WRITE);
  if (error != SP_OK) {
    std::cerr << "Could not open port.\n"
              << sp_last_error_message() << '\n';
    exit(1);
  }

  // Setup port
  sp_set_baudrate(port, 9600);
  sp_set_bits(port, 8);
  sp_set_parity(port, SP_PARITY_NONE);
  sp_set_stopbits(port, 1);

  do {
    // Read command from user
    char buffer[32];
    std::cout << "\n> "; // prompt
    std::cin.getline(buffer, sizeof(buffer));
    uint8_t cmd;
    uint8_t arg;
    if (strncmp(buffer, "EXIT", 4) == 0) {
      std::cout << "Exit\n";
      exit(0);
    } else if (strncmp(buffer, "ON", 2) == 0) {
      cmd = Command::On;
      arg = std::atoi(buffer+2);
    } else if (strncmp(buffer, "OFF", 3) == 0) {
      cmd = Command::Off;
      arg = std::atoi(buffer+3);
    } else if (strncmp(buffer, "TOGGLE", 6) == 0) {
      cmd = Command::Toggle;
      arg = std::atoi(buffer+6);
    } else if (strncmp(buffer, "STATE", 5) == 0) {
      cmd = Command::State;
      arg = std::atoi(buffer+5);
    } else {
      std::cout << "Commands:\n"
                << "  ON led          Turn led on\n"
                << "  OFF led         Turn led off\n"
                << "  TOGGLE led      Toggle led\n"
                << "  STATE led       Show state of led\n"
                << "  HELP            Print this help\n"
                << std::endl;
      continue;
    }

    // Send command
    buffer[0] = cmd;
    buffer[1] = arg - 1;
    sp_blocking_write(port, buffer, 2, 0);

    // Read string from port
    std::string reply = read_string(port);

    std::cout << reply << '\n';
  } while (std::cin);

  // Cleanup
  sp_close(port);
  sp_free_port(port);
  return 0;
}
