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

.PHONY: setup validate compile upload logs clean idedata-guard

setup:
	uv sync

# Both boards share one ESPHome build cache, so a stale one can flash the wrong
# board's firmware. Delete it unless it was built for $(BOARD).
idedata-guard:
	@for f in .esphome/idedata/*.json; do \
		[ -e "$$f" ] || continue; \
		grep -q -- "-$(BOARD)/" "$$f" || rm -f "$$f"; \
	done

validate: setup
	$(ESPHOME) config $(CONFIG)

compile: setup idedata-guard
	$(ESPHOME) compile $(CONFIG)

upload: setup idedata-guard
	$(ESPHOME) upload $(CONFIG) --device $(DEVICE)

logs: setup idedata-guard
	$(ESPHOME) logs $(CONFIG) --device $(DEVICE)

clean:
	rm -rf .esphome build
