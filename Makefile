# ─────────────────────────────────────────────────────────
# SupportHive-C — Developer Makefile
# Kirov Dynamics Engineering Standard
# ─────────────────────────────────────────────────────────

.PHONY: help build clean test lint docker-up docker-down logs

help:
	@echo ""
	@echo "  SupportHive-C — Available Commands"
	@echo "  ─────────────────────────────────────────────"
	@echo "  make build        Configure and build the project (CMake)"
	@echo "  make test         Run unit tests (CTest)"
	@echo "  make lint         Run static analysis (cppcheck/clang-tidy)"
	@echo "  make clean        Remove build artifacts"
	@echo "  make docker-up    Start performance monitoring stack"
	@echo "  make docker-down  Tear down docker stack"
	@echo ""

# ─── Build ───────────────────────────────────────────────
build:
	@echo "→ Configuring CMake..."
	cmake -B build -S .
	@echo "→ Building project..."
	cmake --build build --config Release

# ─── Testing ─────────────────────────────────────────────
test:
	@echo "→ Running CTests..."
	cd build && ctest --output-on-failure

# ─── Linting ─────────────────────────────────────────────
lint:
	@echo "→ Running static analysis..."
	cppcheck --enable=all --suppress=missingIncludeSystem src/ include/

# ─── Cleanup ─────────────────────────────────────────────
clean:
	@echo "→ Removing build directory..."
	powershell -Command "if (Test-Path build) { Remove-Item -Recurse -Force build }"
	@echo "✅ Cleaned."

# ─── Docker ──────────────────────────────────────────────
docker-up:
	docker-compose up -d
	@echo "✅ Performance dashboard running."

docker-down:
	docker-compose down
	@echo "🛑 Stack stopped."
