# Monitor Jakości Powietrza – JPO 2025/2026

Aplikacja pobiera dane pomiarowe z bezpłatnego REST API Głównego Inspektoratu Ochrony Środowiska (GIOŚ) i prezentuje je w formie wykresów, tabel analitycznych oraz mapy geograficznej z naniesionymi stacjami pomiarowymi.

## Wymagania

- Qt 6.5 lub nowszy (Core, Gui, Widgets, Network, Charts)
- CMake 3.20+
- Kompilator C++17 (MSVC 2022+, GCC 11+, Clang 14+)
- Git (wymagany przez FetchContent)
- Połączenie z internetem przy pierwszej konfiguracji CMake  
  *(pobierane automatycznie: nlohmann/json v3.11.3, Google Test v1.14.0)*

## Budowanie

### Windows

Otworzyć: **x64 Native Tools Command Prompt for VS 2022**

```bash
# Klonowanie repozytorium
git clone <url> AirQualityMonitor
cd AirQualityMonitor

# Konfiguracja CMake (podać własną ścieżkę do Qt)
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"

# Kompilacja
cmake --build build --parallel --config Release

# Kopiowanie DLL Qt
C:\Qt\6.x.x\msvc2022_64\bin\windeployqt.exe build\AirQualityMonitor.exe

# Uruchomienie
build\AirQualityMonitor.exe
```

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/AirQualityMonitor
```

## Uruchamianie testów

```bash
# Windows
cmake --build build --parallel --config Release
cd build
ctest --output-on-failure -C Release

# Linux / macOS
cmake --build build --parallel
cd build
ctest --output-on-failure
```

## Generowanie dokumentacji

Wymagany [Doxygen](https://www.doxygen.nl).

```bash
doxygen Doxyfile
```

Dokumentacja HTML: `docs/html/index.html`

## Obsługa aplikacji

### Zakładka „Stacje"
- Kliknij **Załaduj wszystkie stacje** – pobiera pełną listę z API GIOŚ
- Wpisz miasto i kliknij **Filtruj po mieście**
- Wpisz miasto i adres, ustaw promień i kliknij **Szukaj w promieniu**  
  *(geokodowanie przez OpenStreetMap Nominatim)*  
  > **Uwaga:** pole Miasto musi być wypełnione przed wyszukiwaniem w promieniu
- Dwuklik na stacji → przejście do zakładki Stanowiska

### Zakładka „Stanowiska"
- Kliknij stanowisko (czujnik) aby je wybrać
- **Pobierz pomiary z API** – pobiera najnowsze dane z GIOŚ (ostatnie 3 doby)
- **Zapisz w bazie lokalnej** – zapisuje pobrane dane do pliku JSON
- **Wczytaj z bazy lokalnej** – odczytuje zapisane dane bez połączenia z siecią

> **Uwaga:** stanowiska manualne nie mają danych bieżących (dostępne po 4–8 tygodniach)

### Zakładka „Wykres"
- Interaktywny wykres liniowy danych pomiarowych (Qt Charts)
- Wybierz zakres dat **Od / Do** i kliknij **Zastosuj**
- Przycisk **Cały zakres** przywraca pełny dostępny zakres danych

### Zakładka „Analiza"
- Wyświetla: wartość minimalną i maksymalną (z datami), średnią i trend
- Wybierz zakres dat i kliknij **Analizuj**
- Zmiana zakresu automatycznie aktualizuje wykres

### Zakładka „Mapa"
- Mapa Polski z naniesionymi punktami stacji pomiarowych
- Kolor punktu = indeks jakości powietrza (legenda po prawej stronie)
- Kliknij punkt → stacja zostaje wybrana
- Scroll = zoom, przeciągnij = przesuwanie mapy

## Lokalna baza danych

Pliki JSON zapisywane są automatycznie w:

| System | Ścieżka |
|--------|---------|
| Windows | `%APPDATA%\JPO2025\AirQualityMonitor\db\` |
| Linux/macOS | `~/.local/share/JPO2025/AirQualityMonitor/db/` |

Struktura plików:
- `stations.json` – lista wszystkich stacji
- `sensors_<id>.json` – stanowiska stacji o podanym id
- `measurements_<id>.json` – pomiary stanowiska o podanym id

## Wzorce projektowe

| Wzorzec | Zastosowanie |
|---------|-------------|
| Singleton | `DataController` – jeden kontroler dla całej aplikacji |
| Strategy | `IDatabase` / `JsonDatabase` – wymienny backend bazy danych |
| Observer | Sygnały/sloty Qt – `DataController` → GUI |

## Architektura

```
src/
├── api/       # komunikacja HTTP i parsowanie JSON (GIOŚ REST API v1)
├── model/     # struktury danych (Station, Sensor, Measurement, AQI)
├── core/      # logika biznesowa (DataController, Analyzer)
├── storage/   # lokalna baza danych (IDatabase, JsonDatabase)
└── gui/       # interfejs graficzny Qt Widgets
tests/         # testy jednostkowe (Google Test)
```

## Uwagi

- Aplikacja działa w trybie offline korzystając z danych zapisanych w lokalnej bazie JSON
- Geokodowanie adresów wymaga połączenia z Nominatim (openstreetmap.org)
- Dane pomiarowe dostępne są za ostatnie 3 doby (ograniczenie API GIOŚ)

## Autor

Kacper Kozłowski
