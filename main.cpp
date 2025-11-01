#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <cmath>

using namespace std;

struct Destino {
    string nombre;
    int temperatura;
    int altura;
    vector<string> actividades;
    map<string, float> distancias;
    vector<Destino*> conexiones;
};

struct NodoPrioridad {
    string nombre;
    float distancia;

    bool operator>(const NodoPrioridad& otro) const {
        return distancia > otro.distancia;
    }
};

map<string, Destino*> destinos;

vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


vector<string> dijkstra(const string& origen, const string& destino) {
    // Verificar que origen y destino existan
    if (destinos.find(origen) == destinos.end() || destinos.find(destino) == destinos.end()) {
        cout << "Origen o destino no existen en el grafo." << endl;
        return {};
    }

    unordered_map<string, float> dist;
    unordered_map<string, string> prev;
    set<string> visitados;

    for (const auto& [nombre, _] : destinos) {
        dist[nombre] = numeric_limits<float>::infinity();
    }

    dist[origen] = 0.0;

    priority_queue<NodoPrioridad, vector<NodoPrioridad>, greater<NodoPrioridad>> pq;
    pq.push({origen, 0.0});

    while (!pq.empty()) {
        NodoPrioridad actualNodo = pq.top();
        pq.pop();
        string actual = actualNodo.nombre;

        if (visitados.count(actual)) continue;
        visitados.insert(actual);

        for (Destino* vecino : destinos[actual]->conexiones) {
            string nombreVecino = vecino->nombre;

            // Verificar que exista la distancia al vecino
            if (destinos[actual]->distancias.find(nombreVecino) == destinos[actual]->distancias.end()) continue;

            float peso = destinos[actual]->distancias[nombreVecino];
            float nuevaDistancia = dist[actual] + peso;

            if (nuevaDistancia < dist[nombreVecino]) {
                dist[nombreVecino] = nuevaDistancia;
                prev[nombreVecino] = actual;
                pq.push({nombreVecino, nuevaDistancia});
            }
        }
    }

    // Reconstruir el camino
    vector<string> camino;
    string actual = destino;

    if (prev.find(actual) == prev.end() && actual != origen) {
        cout << "No hay ruta disponible entre " << origen << " y " << destino << endl;
        return {};
    }

    while (actual != origen) {
        camino.push_back(actual);
        actual = prev[actual];
    }
    camino.push_back(origen);
    reverse(camino.begin(), camino.end());

    // Mostrar distancia total
    cout << "Distancia total: " << dist[destino] << endl;

    return camino;
}


void cargarDestinos(const string& archivo) {
    ifstream file(archivo);
    string linea;
    vector<string> ciudadesBase = {"Medellin", "Rionegro", "Santa Fe de Antioquia", "Barbosa", "Caldas"};

    while (getline(file, linea)) {
        vector<string> partes = split(linea, ',');
        if (partes.size() < 9) continue;

        Destino* d = new Destino();
        d->nombre = partes[0];
        d->temperatura = stoi(partes[1]);
        d->altura = stoi(partes[2]);
        d->actividades = split(partes[3], ';');

        for (int i = 0; i < ciudadesBase.size(); ++i) {
            d->distancias[ciudadesBase[i]] = stof(partes[4 + i]);
        }

        destinos[d->nombre] = d;

        for (int i = 0; i < ciudadesBase.size(); ++i) {
            if (destinos.find(ciudadesBase[i]) == destinos.end() || d->distancias[ciudadesBase[i]] == 0)
                continue;
            d->conexiones.push_back(destinos[ciudadesBase[i]]);
            destinos[ciudadesBase[i]]->conexiones.push_back(d);
            destinos[ciudadesBase[i]]->distancias[d->nombre] = d->distancias[ciudadesBase[i]];
        }
    }
}

float calcularSimilitud(const vector<string>& a1, const vector<string>& a2) {
    set<string> s1(a1.begin(), a1.end());
    set<string> s2(a2.begin(), a2.end());
    vector<string> interseccion;

    for (const auto& act : s1) {
        if (s2.count(act)) {
            interseccion.push_back(act);
        }
    }

    return static_cast<float>(interseccion.size()) / max(s1.size(), s2.size());
}

Destino* recomendarDestino(Destino* favorito, vector<Destino*> yaRecomendados = {}) {
    float mejorPuntaje = -1;
    Destino* mejorDestino = nullptr;

    for (auto& [nombre, destino] : destinos) {
        if (destino == favorito) continue;
        if (find(yaRecomendados.begin(), yaRecomendados.end(), destino) != yaRecomendados.end()) continue;

        float similitud = calcularSimilitud(favorito->actividades, destino->actividades);
        float distancia = destino->distancias[favorito->nombre];
        float puntaje = similitud / (distancia + 1); // +1 para evitar división por cero

        if (puntaje > mejorPuntaje) {
            mejorPuntaje = puntaje;
            mejorDestino = destino;
        }
    }

    return mejorDestino;
}

void mostrarDestino(Destino* d) {
    cout << "Nombre: " << d->nombre << endl;
    cout << "Temperatura: " << d->temperatura << "°C" << endl;
    cout << "Altura: " << d->altura << " m" << endl;
    cout << "Actividades: ";
    for (const auto& act : d->actividades) {
        cout << act << ", ";
    }
    cout << endl;
    cout << "Distancias desde ciudades base:" << endl;
    for (const auto& [ciudad, dist] : d->distancias) {
        cout << "  - " << ciudad << ": " << dist << " km" << endl;
    }
    cout << "-----------------------------" << endl;
}

int main() {
    cargarDestinos("destinos.txt");

    cout << "Opciones disponibles:\n";
    cout << "1. Recomendaciones desde destino favorito\n";
    cout << "2. Calcular ruta mas corta entre dos destinos\n";
    cout << "Seleccione una opcion (1 o 2): ";
    int opcion;
    cin >> opcion;
    cin.ignore();

    if (opcion == 1) {
        string favorito;
        cout << "Ingrese su destino favorito: ";
        getline(cin, favorito);

        if (destinos.find(favorito) == destinos.end()) {
            cout << "Destino no encontrado." << endl;
            return 1;
        }

        Destino* actual = destinos[favorito];
        mostrarDestino(actual);

        cout << "\nRecomendaciones basadas en actividades y cercanía:\n";
        vector<Destino*> recomendados = {actual};
        for (int i = 0; i < 5; ++i) {
            Destino* recomendado = recomendarDestino(actual, recomendados);
            recomendados.push_back(recomendado);
            if (recomendado) {
                cout << "Recomendación #" << i + 1 << ": " << recomendado->nombre << endl;
                mostrarDestino(recomendado);
                actual = recomendado;
            } else {
                cout << "No se encontraron más recomendaciones." << endl;
                break;
            }
        }
    } else if (opcion == 2) {
        string origen, destino;
        cout << "Ingrese el destino de origen: ";
        getline(cin, origen);
        cout << "Ingrese el destino de llegada: ";
        getline(cin, destino);

        if (destinos.find(origen) == destinos.end() || destinos.find(destino) == destinos.end()) {
            cout << "Uno o ambos destinos no existen." << endl;
            return 1;
        }

        vector<string> ruta = dijkstra(origen, destino);
        if (!ruta.empty()) {
            cout << "Ruta más corta de " << origen << " a " << destino << ":\n";
            for (const auto& paso : ruta) {
                cout << paso;
                if (paso != destino) cout << " -> ";
            }
            //cout << "\nDistancia total: " << destinos[destino]->distancias[ruta[ruta.size() - 2]] << " km (último tramo)\n";
        }
    } else {
        cout << "Opción inválida." << endl;
    }

    return 0;
}