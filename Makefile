# cpp_mdnn explores a deep neural network using the MiniDNN library
# to analyse the Adult dataset, aiming to predict income levels based
# on various demographic features.
#
# Copyright (c) 2024 Finbarrs Oketunji
# Written by Finbarrs Oketunji <f@finbarrs.eu>
#
# This file is part of cpp_mdnn.
#
# cpp_mdnn is an open-source software: you are free to redistribute
# and/or modify it under the terms specified in version 3 of the GNU
# General Public License, as published by the Free Software Foundation.
#
# cpp_mdnn is is made available with the hope that it will be beneficial,
# but it comes with NO WARRANTY whatsoever. This includes, but is not limited
# to, any implied warranties of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE. For more detailed information, please refer to the
# GNU General Public License.
#
# You should have received a copy of the GNU General Public License
# along with cpp_mdnn.  If not, visit <http://www.gnu.org/licenses/>.

# Detect the operating system
UNAME_S := $(shell uname -s)

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -O2

# MiniDNN path
MINIDNN_PATH = $(HOME)/MiniDNN

# Include paths
EIGEN_INCLUDE = -I/usr/local/include/eigen3
MINIDNN_INCLUDE = -I$(MINIDNN_PATH)/include

# Source file
SRC = cpp_mdnn.cpp

# Output executable
TARGET = cpp_mdnn

# Results file
RESULT_FILE = RESULT.md

# OS-specific configurations
ifeq ($(UNAME_S),Darwin)
    # macOS configuration
    BREW := $(shell command -v brew 2> /dev/null)
    ifndef BREW
        $(error Homebrew is not installed. Please install Homebrew first.)
    endif
else ifeq ($(UNAME_S),Linux)
    # Linux configuration
    APT := $(shell command -v apt 2> /dev/null)
    ifndef APT
        $(error apt is not available. Please use a Debian-based distribution or install the required packages manually.)
    endif
else
    $(error Unsupported operating system: $(UNAME_S))
endif

all: $(TARGET)

$(TARGET): $(SRC) | minidnn eigen
	$(CXX) $(CXXFLAGS) $(EIGEN_INCLUDE) $(MINIDNN_INCLUDE) -o $@ $< $(LDFLAGS)

run: $(TARGET) ## Run the program and save results
	@echo "# CPP_MDNN Results" > $(RESULT_FILE)
	@echo "\nDate: $$(date)" >> $(RESULT_FILE)
	@echo "\nOperating System: $(UNAME_S)" >> $(RESULT_FILE)
	@echo "\n## Program Output" >> $(RESULT_FILE)
	@echo '```' >> $(RESULT_FILE)
	@./$(TARGET) | tee -a $(RESULT_FILE)
	@echo '```' >> $(RESULT_FILE)
	@echo "\nResults have been saved to $(RESULT_FILE)"

clean: ## Remove the executable and results file
	rm -f $(TARGET) $(RESULT_FILE)

minidnn: ## Clone MiniDNN repository
	@if [ ! -d "$(MINIDNN_PATH)" ]; then \
		git clone git@github.com:yixuan/MiniDNN.git $(MINIDNN_PATH); \
	else \
		echo "MiniDNN already cloned. Updating..."; \
		cd $(MINIDNN_PATH) && git pull; \
	fi

eigen: ## Install Eigen
ifeq ($(UNAME_S),Darwin)
	@if ! brew list eigen > /dev/null; then \
		echo "Installing Eigen..."; \
		brew install eigen; \
	else \
		echo "Eigen is already installed."; \
	fi
else ifeq ($(UNAME_S),Linux)
	@if ! dpkg -s libeigen3-dev > /dev/null 2>&1; then \
		echo "Installing Eigen..."; \
		sudo apt update && sudo apt install -y libeigen3-dev; \
	else \
		echo "Eigen is already installed."; \
	fi
endif

help: ## Display help message
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

.PHONY: all minidnn eigen run clean help
.DEFAULT_GOAL := help
