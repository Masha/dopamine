# Split presets — заметка для бэка (клиент Dopamine)

Клиент с этой версии мержит каталог **builtin ∪ API** по одинаковому `id`.
Офлайн/первый запуск работают без `/v1/split_presets`; новые домены с бека
подтягиваются без релиза аппки.

Снимок вашего прода для сверки: `frkn-services/docs/split-presets.md` (2026-09-25).

## Контракт merge

| | |
|---|---|
| Ключ | `presets[].id` (стабильный, никогда не переиспользовать под другой сервис) |
| Правило | `domains_effective = unique(builtin_domains + api_domains)` |
| Имя | если с API пришло непустое `name` — оно; иначе builtin |
| Cache | на диск клиент пишет **только** ответ API (без builtin) |
| CIDR | можно в `domains` (как у `vk`) — клиент понимает |

**Важно:** тот же `id`, что в builtin. Не публикуйте `builtin-youtube` — только `youtube`.

## Builtin id на клиенте (не дублировать другими смыслами)

### FRKN-only (на беке таких id быть не должно)

| id | Назначение |
|---|---|
| `builtin-ru-direct` | RU services (CIDR + домены) |
| `builtin-ru-banking` | Online Banking |
| `builtin-ru-vpn` | Blocked in RU |
| `builtin-ai` | AI-пачка (ChatGPT/Claude/Gemini/…) |

### Именованные сервисы (тот же id, что в API → union)

| id | База в аппке |
|---|---|
| `youtube` | youtube.com, googlevideo.com, i.ytimg.com, img.youtube.com, youtube-nocookie.com, youtubei.googleapis.com, youtube.googleapis.com |
| `instagram` | instagram.com, static.cdninstagram.com, scontent.cdninstagram.com, ig.me |
| `tiktok` | tiktok.com, tiktokv.com, sf16.tiktokcdn.com, p16.tiktokcdn.com, musical.ly |
| `x` | x.com, twitter.com, pbs/abs/video.twimg.com, t.co |
| `facebook` | facebook.com, fb.com, fbcdn.net, fbsbx.com |
| `whatsapp` | whatsapp.com, whatsapp.net |
| `telegram` | telegram.org, t.me, telegra.ph, telesco.pe, cdn4.telegram.org |
| `netflix` | netflix.com, nflxvideo.net, dnm/art-s.nflximg.net, nflxext.com |
| `spotify` | spotify.com, i.scdn.co, audio-fa.scdn.co |
| `discord` | discord.com, discord.gg, discordapp.com, discordcdn.com |

Как обновлять без релиза: upsert того же `id` и **добавьте только новые** хосты/CIDR
(полную копию базы слать можно — клиент дедупит).

## Что сделать на беке сейчас

### 1. Удалить `twinby`

Снести пресет `twinby` из `mrkting.split_presets` (и сидов). В клиенте его нет и не будет.

```bash
curl -sS -X DELETE -H "Authorization: Bearer $TOKEN" \
  http://127.0.0.1:9103/split_presets/twinby
```

Обновить `frkn-services/docs/split-presets.md` и миграции.

### 2. Оставить (нужны для hot-update / нет полного builtin)

| id | Зачем на беке |
|---|---|
| `youtube`, `instagram`, `tiktok`, `x`, `facebook`, `whatsapp`, `telegram`, `netflix`, `spotify`, `discord` | дельта поверх builtin |
| `vk` | нет builtin; плюс CIDR |
| `erudit` | только бек |
| `ozon`, `wildberries`, `kinopoisk`, `yandex`, `gosuslugi` | узкие тогглы; частично пересекаются с `builtin-ru-direct` — пока оставить, если нужны отдельные галочки |

### 3. По желанию упростить

| id | Комментарий |
|---|---|
| `chatgpt`, `gemini` | перекрываются `builtin-ai`; можно держать как узкие тогглы или выпилить, когда старых клиентов мало |

Не заводить на беке id `builtin-*`.

## Пример upsert (новый CDN у YouTube)

```bash
curl -sS -X POST http://127.0.0.1:9103/split_presets \
  -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' \
  -d '{
    "id": "youtube",
    "name": "YouTube",
    "domains": [
      "youtube.com",
      "googlevideo.com",
      "i.ytimg.com",
      "img.youtube.com",
      "youtube-nocookie.com",
      "youtubei.googleapis.com",
      "youtube.googleapis.com",
      "new-cdn.example.googlevideo.com"
    ],
    "active": true
  }'
```

Клиент после fetch: база из аппки ∪ `new-cdn…` без обновления стора.
