# Split preset: Эрудит

Backend catalog entry for `/v1/split_presets`. Confirm which App Store app the user has (two different products share the name).

## A — «Эрудит с друзьями» (most common in RU)

- iOS bundle: `ru.mail.games.erudit`
- Android: `ru.mail.games.android.Erudit`
- Site: https://erugame.ru/
- Developer on store: often listed under Mail.ru / private publisher

```json
{
  "id": "erudit",
  "name": "Эрудит",
  "description": "Эрудит с друзьями (erugame.ru)",
  "domains": [
    "erugame.ru",
    "www.erugame.ru",
    "game.erugame.ru",
    "new.erugame.ru",
    "cdn.erugame.ru"
  ]
}
```

Do **not** add bare `mail.ru` / `ok.ru` / `vk.com` — social login would pull half the internet into the preset. If login via VK/OK breaks under VPN, use existing social/media presets or a manual site, not this game bundle.

Optional (broader Mail.ru Games platform — only if online lobby still fails after the core list):

```text
games.mail.ru
api.games.mail.ru
```

## B — «Эрудит — игра в слова» (UA-PLAY)

- iOS/Android: `com.uaplay.scrabbleua`
- Site: https://games.ua-play.com
- Privacy: https://erudite.ua-play.com/privacy-policy.html

```json
{
  "id": "erudit-uaplay",
  "name": "Erudite (UA-PLAY)",
  "description": "Эрудит UA-PLAY",
  "domains": [
    "ua-play.com",
    "games.ua-play.com",
    "erudite.ua-play.com",
    "api.ua-play.com"
  ]
}
```

## How to verify on iPhone

1. Settings → App Store listing → developer / support URL, or Settings → General → iPhone Storage → app name.
2. Or with VPN off: play one online move, then in a packet capture / DNS log (or Mac sharing + `tcpdump`) confirm hosts — extend the list if something else appears (push, ads CDN).

## Suggested catalog pick

Ship **A** as `erudit` for RU users. Add **B** only if someone reports the UA-PLAY build.
