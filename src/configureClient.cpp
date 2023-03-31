#include "main.hpp"

void configureClient(Aws::Client::ClientConfiguration &clientConfig,
                     Aws::String &endpointOverride,
                     Aws::String &region) {
  clientConfig.endpointOverride = endpointOverride;
  clientConfig.region = region;
}
