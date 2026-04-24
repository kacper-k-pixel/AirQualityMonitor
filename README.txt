========================================================
  Monitor Jakości Powietrza – JPO 2025/2026
========================================================

OPIS
----
Aplikacja pobiera dane pomiarowe z bezpłatnego REST API
Głównego Inspektoratu Ochrony Środowiska (GIOŚ) i prezentuje
je w formie wykresów, tabel analitycznych oraz uproszczonej
mapy geograficznej.

WYMAGANIA
---------
  - Qt 6.5 lub nowszy (Core, Gui, Widgets, Network, Charts)
  - CMake 3.20+
  - Kompilator C++17 (GCC 11+, Clang 14+, MSVC 2022+)
  - Git (wymagany przez FetchContent)
  - Połączenie z internetem przy pierwszej konfiguracji CMake
    (pobierane: nlohmann/json v3.11.3, Google Test v1.14.0)

BUDOWANIE
---------
  # Klonowanie repozytorium
  git clone <url> AirQualityMonitor
  cd AirQualityMonitor

  # Konfiguracja i kompilacja (Release)
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --parallel

  # Uruchomienie aplikacji
  ./build/AirQualityMonitor          # Linux / macOS
  build\Release\AirQualityMonitor.exe  # Windows

URUCHAMIANIE TESTÓW
-------------------
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build --parallel
  cd build
  ctest --output-on-failure

  Lub bezpośrednio:
    ./tests/test_JsonParser
    ./tests/test_Analyzer
    ./tests/test_JsonDatabase

GENEROWANIE DOKUMENTACJI
------------------------
  Wymagany Doxygen (https://www.doxygen.nl)

  doxygen Doxyfile

  Dokumentacja HTML: docs/html/index.html

OBSŁUGA APLIKACJI
-----------------
1. Zakładka „Stacje":
   - Kliknij „Załaduj wszystkie stacje" – pobiera pełną listę z API.
   - Wpisz miasto i kliknij „Filtruj po mieście".
   - Wpisz adres, ustaw promień i kliknij „Szukaj w promieniu"
     (geokodowanie przez OpenStreetMap Nominatim).
   - Dwuklik na stacji → przejście do zakładki Stanowiska.

2. Zakładka „Stanowiska":
   - Wybierz stanowisko (czujnik).
   - „Pobierz pomiary z API" – pobiera najnowsze dane z GIOŚ.
   - „Zapisz w bazie lokalnej" – zapisuje do pliku JSON.
   - „Wczytaj z bazy lokalnej" – odczytuje bez połączenia z siecią.

3. Zakładka „Wykres":
   - Interaktywny wykres liniowy (Qt Charts).
   - Zakres dat sterowany z zakładki Analiza.

4. Zakładka „Analiza":
   - Min, max (z datami), średnia, trend (rosnący / malejący / stabilny).
   - Wybierz zakres dat → wykres aktualizuje się automatycznie.

5. Zakładka „Mapa":
   - Punkty stacji na uproszczonej mapie Polski.
   - Kolor punktu = indeks jakości powietrza (skala 0–5).
   - Kliknij punkt → stacja zostaje wybrana.
   - Scroll = zoom, przeciągnij = przesuwanie.

LOKALNA BAZA DANYCH
-------------------
Pliki JSON zapisywane są w:
  Linux/macOS: ~/.local/share/JPO2025/AirQualityMonitor/db/
  Windows:     %APPDATA%\JPO2025\AirQualityMonitor\db\

Struktura plików:
  stations.json            – lista wszystkich stacji
  sensors_<id>.json        – stanowiska stacji o podanym id
  measurements_<id>.json   – pomiary stanowiska o podanym id

WZORCE PROJEKTOWE
-----------------
  Singleton  – DataController (jeden kontroler dla całej aplikacji)
  Strategy   – IDatabase / JsonDatabase (wymienny backend bazy)
  Observer   – sygnały/sloty Qt (DataController → GUI)

ARCHITEKTURA
------------
  src/api/       – komunikacja HTTP i parsowanie JSON
  src/model/     – struktury danych (Station, Sensor, Measurement, AQI)
  src/core/      – logika biznesowa (DataController, Analyzer)
  src/storage/   – lokalna baza danych (IDatabase, JsonDatabase)
  src/gui/       – interfejs graficzny (Qt Widgets)
  tests/         – testy jednostkowe (Google Test)

UWAGI
-----
- Aplikacja działa w trybie offline korzystając z danych historycznych
  zapisanych w lokalnej bazie JSON.
- Geokodowanie wymaga połączenia z Nominatim (openstreetmap.org).
- Mapa stacji jest uproszczona (QGraphicsView). Pełna mapa wektorowa
  wymagałaby modułu Qt Location lub WebEngine.

AUTOR
-----
  Kacper Kozłowski
