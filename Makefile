CONFIG ?= plant-watering.yaml
DEVICE ?= /dev/ttyACM0
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
