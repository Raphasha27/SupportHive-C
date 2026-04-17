# CONTRIBUTING to Kirov Dynamics

This document defines the engineering workflow standard for all repositories under the Kirov Dynamics organization.

---

## Branch Naming

| Type     | Pattern                             | Example                        |
|----------|-------------------------------------|--------------------------------|
| Feature  | `feature/[issue-#]-[description]`  | `feature/42-jwt-refresh`       |
| Bug Fix  | `fix/[issue-#]-[description]`      | `fix/89-null-session-crash`    |
| Refactor | `refactor/[issue-#]-[description]` | `refactor/12-api-layer`        |
| Chore    | `chore/[description]`              | `chore/upgrade-dependencies`   |
| Docs     | `docs/[description]`               | `docs/deployment-guide`        |

---

## Commit Messages (Conventional Commits — Mandatory)

```text
<type>(<scope>): <short description>
```

**Types:** `feat`, `fix`, `refactor`, `perf`, `test`, `docs`, `chore`, `style`

**Examples:**

```text
feat(auth): implement RSA-based key rotation
fix(api): handle missing authorization header
refactor(core): extract event bus into separate module
test(payments): add integration tests for SLA breach detection
docs(readme): update architecture diagram
chore(ci): pin CodeQL action to v3
```

---

## Pull Request Rules

1. **No direct commits to `main`** — all changes go through a PR.
2. Every PR must be linked to a GitHub Issue (`Closes #42`).
3. PRs must pass CI before merge (tests, lint, security scan).
4. Self-review is required even for solo work — read your own diff.

---

## Code Standards

- **Python:** Follow PEP 8. Use `flake8` and `black` for formatting.
- **TypeScript/JavaScript:** ESLint + Prettier, strict mode enabled.
- **C/C++:** `clang-format` with the project `.clang-format` config.

---

## Local Development

```bash
# Clone the repository
git clone https://github.com/Kirov-Dynamics/<repo-name>.git

# Start full stack locally
docker-compose up --build

# Backend only
cd backend && pip install -r requirements.txt && uvicorn main:app --reload

# Frontend only
cd frontend && npm install && npm run dev
```
