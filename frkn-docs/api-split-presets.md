# Split-tunneling service presets — `POST /v1/split_presets`

Backend spec for the split-tunneling **service presets** feature in the Dopamine
client. Instead of typing domains by hand, the user ticks services
(«YouTube», «ChatGPT», «Gemini», «Instagram»…) and the client adds/removes the
whole domain bundle of that service to the split-tunneling site list.

Presets are delivered **dynamically from the backend** so the bundles can be
updated without an app release. The client caches the last successful response
for offline use.

## Endpoint

```
POST https://api.frkn.org/v1/split_presets
Content-Type: application/json
X-Client-Request-ID: <uuid4>
```

No auth token. Client timeout: **12 s**.

## Transport — AGW encrypted envelope

Identical to `/v1/news` (see `frkn-docs/api-news.md`): outer
`{keyPayload, apiPayload}`, AES-256-CBC payload, RSA-wrapped keys, response =
base64 AES ciphertext with the same key/IV/salt. An unencrypted body is a
decryption error on the client.

## Request payload (decrypted)

```json
{
  "locale": "ru",
  "presets_version": "2026-07-30.1"
}
```

| Field | Type | Presence | Notes |
|---|---|---|---|
| `locale` | string | always | App language, ISO 639-1: `en`, `ru`, `uk`. Fall back to `en`. Used only for preset display names. |
| `presets_version` | string | optional | The `version` of the client's cached preset list. Absent on first fetch. |

If `presets_version` matches the current version on the backend, the backend
MAY answer with `{ "version": "...", "presets": [] }` and the client keeps its
cache. Returning the full list every time is also fine — the payload is small.

## Response payload (decrypted) — 200 OK

```json
{
  "version": "2026-07-30.1",
  "presets": [
    {
      "id": "youtube",
      "name": "YouTube",
      "domains": [
        "youtube.com",
        "googlevideo.com",
        "ytimg.com",
        "youtube-nocookie.com",
        "youtubei.googleapis.com",
        "youtube.googleapis.com"
      ]
    },
    {
      "id": "chatgpt",
      "name": "ChatGPT",
      "domains": [
        "chatgpt.com",
        "openai.com",
        "oaistatic.com",
        "oaiusercontent.com",
        "chat.com"
      ]
    },
    {
      "id": "gemini",
      "name": "Gemini",
      "domains": [
        "gemini.google.com",
        "bard.google.com",
        "gstatic.com"
      ]
    },
    {
      "id": "instagram",
      "name": "Instagram",
      "domains": [
        "instagram.com",
        "cdninstagram.com",
        "ig.me"
      ]
    }
  ]
}
```

Fields:

| Field | Type | Required | Notes |
|---|---|---|---|
| `version` | string | yes | Any string that **changes whenever the list changes** (date-based is fine). The client refetches and reapplies on change. |
| `presets[].id` | string | yes | Stable unique id (`youtube`, `chatgpt`, …). **Never reuse** for a different service — the client stores toggle state per id. |
| `presets[].name` | string | yes | Display name (brand names need no real localization; still returned per `locale`). |
| `presets[].description` | string | no | Short hint under the name (e.g. which AIs are inside a combined preset). Shown in small muted type. |
| `presets[].domains` | array of strings | yes | Plain host suffixes, **no scheme, no `*.` prefix**. A domain matches itself and all subdomains. |

Domain semantics (must match the client's site list):

- Hostnames preferred; CIDRs are allowed (e.g. VK ranges) and applied as
  routes without DNS.
- Keep bundles conservative: a preset must not accidentally cover half the
  internet (avoid `google.com`, `cloudflare.com` etc.). Prefer specific
  service domains over shared corporate parents.

## Builtin ∪ API merge (client)

The Dopamine client ships offline baselines for major services and FRKN
routing packs. At fetch/connect time it builds:

`domains_effective = unique(builtin_domains ∪ api_domains)` for the same `id`.

| Builtin id | Notes |
|---|---|
| `builtin-ru-direct`, `builtin-ru-banking`, `builtin-ru-vpn`, `builtin-ai` | Client-only. **Do not** publish these ids from the API. |
| `youtube`, `instagram`, `tiktok`, `x`, `facebook`, `whatsapp`, `telegram`, `netflix`, `spotify`, `discord` | Same ids on API → union. Publish **deltas** (new hosts/CIDRs) under the same id; full lists are fine (client dedupes). |

Do **not** invent parallel ids (`builtin-youtube`). Do **not** ship `twinby`
(remove from prod if present).

Details for the backend team: `frkn-docs/split-presets-backend.md`.

Semantics:

- Empty list → `{ "version": "...", "presets": [] }` with HTTP 200 is the
  normal «nothing configured» case, not an error (client still shows builtins).
- Toggle state (`preset id → on/off`) is stored locally per device.
- API catalog order is preserved for API rows; client-only builtins are listed
  above. Cache stores the API payload only (builtins are merged in memory).
- Cache-friendly: silent fetch on app start / when opening split-tunneling settings.

## Errors

| Situation | Recommended response | Client behavior |
|---|---|---|
| No presets configured | `200` + `{"version": "...", "presets": []}` | Presets section hidden, manual list unchanged. |
| Backend failure | `500` (+ encrypted error body) | Silent on background fetch — cached/last list used. |

Avoid `404` / `409` / `501` (special semantics in other client flows).

## Initial preset set (suggested)

| id | name | Notes |
|---|---|---|
| `youtube` | YouTube | see example above |
| `chatgpt` | ChatGPT | see example above |
| `gemini` | Gemini | see example above |
| `ai` | AI | Combined bundle: ChatGPT, Claude, Gemini, Perplexity, DeepSeek, Grok. Optional `description` lists the brands. Prefer this over shipping six separate AI toggles for most users. |
| `erudit` | Эрудит | API-only. See `frkn-docs/split-preset-erudit.md`. |
| `instagram` … `discord` | (major services) | Same ids as client builtins → union. |
| `vk` | VK | API-only (domains + CIDRs). |
| ~~`twinby`~~ | — | **Remove** from prod. |

Review each bundle against the «conservative» rule above before publishing.

See `frkn-docs/split-presets-backend.md` for the full backend checklist.

## Client flow (for context)

1. App start with a subscription server → silent fetch; failure ignored,
   cached list used.
2. Settings → Split tunneling → presets section with checkboxes on top of the
   manual site list; toggling applies the bundle into the site list immediately.
3. Toggle state (`preset id → on/off`) is stored locally per device; the
   backend does not need per-user state.
