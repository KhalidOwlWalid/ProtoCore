This library contains implementation of different protocols and its abstraction to allow for ease of integration for your application.

# Folder structure
protocore/  
├── include/                # Public headers (API exposed to users)  
│   └── protocore/          # Namespace-mirrored headers (e.g., `#include <protocore/tcp.hpp>`)  
│       ├── core/           # Core abstractions (e.g., Protocol base class, interfaces)  
│       ├── drivers/        # Hardware/OS-specific implementations (UART, CAN, etc.)  
│       ├── protocols/      # Protocol implementations (TCP, UDP, custom...)  
│       └── utils/          # Utilities (logging, serialization, buffers)  
├── src/                    # Private implementation  
│   ├── core/               # Core logic implementations  
│   ├── drivers/            # Driver implementations (e.g., Linux UART, SocketCAN)  
│   ├── protocols/          # Protocol logic (e.g., TCP state machine)  
│   └── utils/              # Internal helpers  
├── examples/               # Example usage  
│   ├── basic/              # Minimal examples (e.g., echo server)  
│   ├── advanced/           # Complex demos (multi-protocol routing)  
├── tests/                  # Unit/integration tests  
├── docs/                   # Doxygen, markdown guides
└── external/               # Submodules or vendored dependencies (if any) 