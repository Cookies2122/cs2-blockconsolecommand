<h1 align=center><code>Block Console Command</code></h1>

<div align=center>
  <img src=https://img.shields.io/badge/Counter--Strike%202-Metamod%202.0%20(KHook)-orange?style=flat-square alt=metamod>
  <a href=https://github.com/Cookies2122/cs2-blockconsolecommand/commits><img src=https://img.shields.io/github/last-commit/Cookies2122/cs2-blockconsolecommand?style=flat-square alt="last commit"></a>
  <a href=LICENSE><img src=https://img.shields.io/github/license/Cookies2122/cs2-blockconsolecommand?style=flat-square alt=license></a>
</div>
<br>

Запрещает игрокам консольные команды из списка, по умолчанию `meta` и `css`, чтобы не светить список плагинов.

> Автор плагина: [ABKAM](https://github.com/ABKAM2023). Здесь версия 1.0 под новый SDK и Metamod 2.0, оригинал: [ABKAM2023/cs2-blockconsolecommand](https://github.com/ABKAM2023/cs2-blockconsolecommand).

## Что нужно

- [Metamod:Source](https://github.com/alliedmodders/metamod-source) 2.0 (KHook)
- [Utils](https://github.com/Pisex/cs2-menus)

## Установка

Соберите плагин (раздел [Сборка](#сборка)) и скопируйте содержимое `build/package` в папку `csgo` на сервере. В пакете лежит:

| Файл | Куда |
| --- | --- |
| `BlockConsoleCommand.so` | `addons/BlockConsoleCommand/` |
| `BlockConsoleCommand.vdf` | `addons/metamod/` |
| `blockconsolecommand.txt` | `addons/configs/` |
| `blockconsolecommand.phrases.txt` | `addons/translations/` |

После этого перезапустите сервер или смените карту.

## Настройки

### blockconsolecommand.txt

`addons/configs/blockconsolecommand.txt`

Одна команда на строку.

```
meta
css
```

## Переводы

Файл `addons/translations/blockconsolecommand.phrases.txt`, языки: ru, en, ua. Его можно править под себя.

<details>
<summary>Все фразы</summary>

```
"Phrases"
{
    "CommandBlocked"
    {
        "en"	"This command is blocked."
        "ru"	"Эта команда заблокирована."
        "ua"	"Ця команда заблокована."
    }
}
```

</details>

## Что изменено

По сравнению с оригиналом:

- Переведено на новый SDK ([Wend4r/sourcesdk](https://github.com/Wend4r/sourcesdk)), старый после обновления CS2 от 12.11.2026 уже не подходит.
- Работает только на Metamod 2.0 (KHook), старый Metamod на SourceHook больше не поддерживается.
- Заголовки Utils обновлены под текущую версию, имена интерфейсов теперь `UTILS_INTERFACE` и `MENUS_INTERFACE`.
- Своя функция `GameEntitySystem()` убрана, в новом SDK entity system берётся через `IGameResourceService`.
- SchemaEntity обновлена: актуальные оффсеты виртуальных функций.
- Хуки переписаны на KHook из Metamod 2.0, без прослоек под старый SourceHook.
- При выгрузке плагин проверяет, что Utils ещё загружен. Раньше `meta unload` после выгрузки Utils мог уронить сервер.

## Сборка

Собирается на Linux. Нужны clang, python3, cmake и [AMBuild](https://github.com/alliedmodders/ambuild). Все команды ниже выполняются из одной папки.

**1. SDK.** Нужен [sourcesdk](https://github.com/Wend4r/sourcesdk) не старее обновления CS2 от 12.11.2026. Собирается один раз:

```bash
git clone --recursive https://github.com/Wend4r/sourcesdk hl2sdk-cs2
cmake -S hl2sdk-cs2 -B hl2sdk-cs2/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DSOURCESDK_AM_DEFINES=ON -DSOURCESDK_USE_MINIMAL_PROTOS=OFF
cmake --build hl2sdk-cs2/build -j$(nproc)
export HL2SDKCS2=$PWD/hl2sdk-cs2
```

**2. Metamod.** Нужен Metamod 2.0 с KHook, ветка `master`. Со старым Metamod на SourceHook сборка остановится с ошибкой.

```bash
git clone --recursive https://github.com/alliedmodders/metamod-source
```

**3. Плагин.** Папка SchemaEntity должна лежать рядом с папкой плагина.

```bash
git clone https://github.com/{o}/SchemaEntity
git clone https://github.com/Cookies2122/cs2-blockconsolecommand
cd cs2-blockconsolecommand
mkdir build && cd build
python3 ../configure.py --enable-optimize --sdks=cs2 --targets=x86_64 --mms_path=$PWD/../../metamod-source --hl2sdk-manifests=../hl2sdk-manifests
ambuild
```

Готовые файлы появятся в `build/package`.

## Автор

Плагин написал [ABKAM](https://github.com/ABKAM2023), оригинал: [ABKAM2023/cs2-blockconsolecommand](https://github.com/ABKAM2023/cs2-blockconsolecommand).
Перенос на новый SDK и Metamod 2.0, исправления: \_ded\_cookies.

## Лицензия

GPL-3.0, текст в файле [LICENSE](LICENSE). В коде плагина лицензия тоже указана как GPL. Авторские права на оригинальный код у ABKAM.
