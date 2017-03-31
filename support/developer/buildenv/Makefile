SRCROOT := $(realpath .)

STAGING_DIR=$(SRCROOT)/staging
DOCKER_IMAGE=$(STAGING_DIR)/lightwave-dev-kit.tar
DOCKER_IMAGE_TAG="vmware/lightwave-dev-kit"

all: container

container: $(DOCKER_IMAGE)
	docker save $(DOCKER_IMAGE_TAG) > $(DOCKER_IMAGE)

$(DOCKER_IMAGE): $(STAGING_DIR)/Dockerfile
	docker build --no-cache --tag $(DOCKER_IMAGE_TAG) $(STAGING_DIR)

$(STAGING_DIR)/Dockerfile: $(STAGING_DIR) Dockerfile
	cp -f Dockerfile $@

$(STAGING_DIR):
	mkdir -p $@

clean:
	rm -rf $(STAGING_DIR)

