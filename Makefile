# Supported boards: atoms3-lite, atom-lite
BOARD ?= atoms3-lite
CONFIG ?= plant-watering-$(BOARD).yaml

# The Atom Lite talks through a CH9102/CP210x bridge; the AtomS3 Lite uses native USB.
DEVICE ?= $(if $(filter atom-lite,$(BOARD)),/dev/ttyUSB0,/dev/ttyACM0)

VENV ?= venv

export UV_PROJECT_ENVIRONMENT := $(VENV)
export UV_CACHE_DIR := $(CURDIR)/.uv-cache
export XDG_CACHE_HOME := $(CURDIR)/$(VENV)/.cache
export PLATFORMIO_CORE_DIR := $(CURDIR)/$(VENV)/platformio

ESPHOME := $(CURDIR)/$(VENV)/bin/esphome

.PHONY: setup validate compile upload logs clean

setup:
	uv sync

validate: setup
	$(ESPHOME) config $(CONFIG)

compile: setup
	$(ESPHOME) compile $(CONFIG)

upload: setup
	$(ESPHOME) upload $(CONFIG) --device $(DEVICE)

logs: setup
	$(ESPHOME) logs $(CONFIG) --device $(DEVICE)

clean:
	rm -rf .esphome build
