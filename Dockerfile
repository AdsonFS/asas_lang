# Etapa 1: Build
FROM ubuntu:22.04 AS builder

# Evita prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Instala dependências para compilação
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Define o diretório de trabalho
WORKDIR /app

# Copia todos os arquivos do projeto para dentro do container
COPY . .

# Cria diretório de build e compila o projeto
RUN mkdir -p build && \
    cmake -S . -B build \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_BUILD_TYPE=Debug -DENABLE_TRACE=OFF \
      -DENABLE_GC_LOGGING=OFF && \
    cmake --build build -- -j$(nproc)

# Etapa 2: Runtime (imagem leve)
FROM ubuntu:22.04 AS runtime

# Instala apenas o necessário para rodar o executável
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Define o diretório de trabalho
WORKDIR /app

# Copia o executável gerado da etapa de build
COPY --from=builder /app/build/asas .

# Define o comando padrão ao iniciar o container
ENTRYPOINT ["./asas"]

