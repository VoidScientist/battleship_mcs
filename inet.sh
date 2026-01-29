#!/bin/bash

show_client_server() {
  watch -n 0.1 "netstat --inet -atp | grep -E 'SERVER|CLIENT'"
}

show_client_server