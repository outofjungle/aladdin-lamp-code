# Makefile for Aladdin Lamp Project
# Provides convenient targets for building, testing, and uploading

.DEFAULT_GOAL := help
.PHONY: help build upload monitor clean test test-native test-embedded all flash size

# ============================================================================
# CONFIGURATION
# ============================================================================

ENV ?= pico32
PORT ?= auto

# ============================================================================
# COLORS FOR OUTPUT
# ============================================================================

COLOR_RESET   = \033[0m
COLOR_BOLD    = \033[1m
COLOR_GREEN   = \033[32m
COLOR_YELLOW  = \033[33m
COLOR_BLUE    = \033[34m
COLOR_CYAN    = \033[36m

# ============================================================================
# HELP TARGET
# ============================================================================

help: ## Show this help message
	@echo "$(COLOR_BOLD)Aladdin Lamp - HomeKit Candle Light$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_CYAN)Usage:$(COLOR_RESET)"
	@echo "  make $(COLOR_GREEN)<target>$(COLOR_RESET)"
	@echo ""
	@echo "$(COLOR_CYAN)Build Targets:$(COLOR_RESET)"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  $(COLOR_GREEN)%-18s$(COLOR_RESET) %s\n", $$1, $$2}'
	@echo ""
	@echo "$(COLOR_CYAN)Examples:$(COLOR_RESET)"
	@echo "  make build          # Build firmware for ESP32"
	@echo "  make upload         # Upload firmware to device"
	@echo "  make flash          # Build and upload in one step"
	@echo "  make test           # Run all tests"
	@echo "  make monitor        # Open serial monitor"
	@echo ""

# ============================================================================
# BUILD TARGETS
# ============================================================================

build: ## Build firmware for ESP32
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Building firmware...$(COLOR_RESET)"
	pio run -e $(ENV)
	@echo "$(COLOR_GREEN)✓ Build complete$(COLOR_RESET)"

clean: ## Clean build artifacts
	@echo "$(COLOR_BOLD)$(COLOR_YELLOW)Cleaning build artifacts...$(COLOR_RESET)"
	pio run --target clean
	@echo "$(COLOR_GREEN)✓ Clean complete$(COLOR_RESET)"

size: ## Show firmware size information
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Firmware size:$(COLOR_RESET)"
	pio run --target size -e $(ENV)

# ============================================================================
# UPLOAD TARGETS
# ============================================================================

upload: ## Upload firmware to ESP32
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Uploading firmware...$(COLOR_RESET)"
	pio run --target upload -e $(ENV)
	@echo "$(COLOR_GREEN)✓ Upload complete$(COLOR_RESET)"

flash: build upload ## Build and upload firmware

# ============================================================================
# MONITOR TARGETS
# ============================================================================

monitor: ## Open serial monitor
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Opening serial monitor (press Ctrl+C to exit)...$(COLOR_RESET)"
	pio device monitor

flash-monitor: flash monitor ## Build, upload, and monitor

# ============================================================================
# TEST TARGETS
# ============================================================================

test: test-native ## Run all tests (native platform)

test-native: ## Run tests on native platform (desktop)
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Running tests on native platform...$(COLOR_RESET)"
	pio test -e test_native
	@echo "$(COLOR_GREEN)✓ Native tests complete$(COLOR_RESET)"

test-embedded: ## Run tests on embedded device
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Running tests on embedded device...$(COLOR_RESET)"
	@echo "$(COLOR_YELLOW)Note: Device must be connected$(COLOR_RESET)"
	pio test -e test_embedded
	@echo "$(COLOR_GREEN)✓ Embedded tests complete$(COLOR_RESET)"

test-all: test-native test-embedded ## Run tests on all platforms

# ============================================================================
# DEVELOPMENT TARGETS
# ============================================================================

all: clean build test ## Clean, build, and test

check: ## Check for issues without building
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Checking code...$(COLOR_RESET)"
	pio check -e $(ENV)

# ============================================================================
# UTILITY TARGETS
# ============================================================================

info: ## Show project information
	@echo "$(COLOR_BOLD)$(COLOR_CYAN)Project Information:$(COLOR_RESET)"
	@echo "  Name:        Aladdin Lamp - HomeKit Candle Light"
	@echo "  Platform:    ESP32 PICO32"
	@echo "  Framework:   Arduino"
	@echo "  Build Env:   $(ENV)"
	@echo ""
	@echo "$(COLOR_BOLD)$(COLOR_CYAN)Dependencies:$(COLOR_RESET)"
	@echo "  FastLED:     ^3.10.3"
	@echo "  HomeSpan:    ^2.1.0"
	@echo ""
	@echo "$(COLOR_BOLD)$(COLOR_CYAN)Hardware:$(COLOR_RESET)"
	@echo "  LED Strips:  2x APA102 (8 LEDs each)"
	@echo "  Strip 1:     GPIO 26 (data), GPIO 25 (clock)"
	@echo "  Strip 2:     GPIO 19 (data), GPIO 18 (clock)"
	@echo "  Power Btn:   GPIO 0"
	@echo "  Reset Btn:   GPIO 39"
	@echo "  Status LED:  GPIO 22"

list: ## List all available PlatformIO environments
	@echo "$(COLOR_BOLD)$(COLOR_CYAN)Available environments:$(COLOR_RESET)"
	@pio run --list-targets | grep "Environment:" || true
	@echo ""
	@echo "$(COLOR_BOLD)Configured environments:$(COLOR_RESET)"
	@echo "  pico32          - Main firmware build"
	@echo "  test_native     - Native platform tests"
	@echo "  test_embedded   - Embedded device tests"

# ============================================================================
# DEBUG TARGETS
# ============================================================================

verbose: ## Build with verbose output
	@echo "$(COLOR_BOLD)$(COLOR_BLUE)Building with verbose output...$(COLOR_RESET)"
	pio run -v -e $(ENV)

# ============================================================================
# HOMEKIT TARGETS
# ============================================================================

setup-wifi: flash-monitor ## Upload and configure WiFi (via serial)
	@echo "$(COLOR_YELLOW)Use serial commands to configure WiFi:$(COLOR_RESET)"
	@echo "  W <ssid> <password> - Configure WiFi"
	@echo "  S - Show status"
	@echo "  A - Start pairing mode"
