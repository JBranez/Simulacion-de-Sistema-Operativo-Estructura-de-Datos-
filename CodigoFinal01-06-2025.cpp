#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <sstream> // Para to_string alternativo
#include <cstdlib>

using namespace std;

/* ================================================================
 *                   FUNCIONES AUXILIARES
 * ================================================================ */
// Implementación alternativa de to_string para compiladores antiguos
template <typename T>
string to_string_alt(T value) {
    ostringstream os;
    os << value;
    return os.str();
}

/* ================================================================
 *                   GESTOR DE ERRORES
 * ================================================================ */
class ErrorHandler {
public:
    static void manejar(const exception& e) {
        cerr << "\n[ERROR] " << e.what() << endl;
        ofstream log("errors.log", ios::app);
        if (log.is_open()) {
            log << "[ERROR] " << e.what() << endl;
            log.close();
        }
    }
};

/* ================================================================
 *                   NODO DE PROCESO
 * ================================================================ */
class NodoProcesso {
public:
    int id;
    string nombre;
    int prioridad;
    NodoProcesso* siguiente;

    NodoProcesso(int id, string nombre, int prioridad) 
        : id(id), nombre(nombre), prioridad(prioridad), siguiente(NULL) {}
};

/* ================================================================
 *                   GESTOR DE PERSISTENCIA
 * ================================================================ */
class Persistencia {
public:
    static void guardarProcesos(NodoProcesso* cabeza, const string& archivo) {
        ofstream file(archivo.c_str());
        if (!file.is_open()) {
            throw runtime_error("No se pudo abrir " + archivo + " para escritura");
        }
        
        NodoProcesso* actual = cabeza;
        while (actual) {
            file << actual->id << "," << actual->nombre << "," << actual->prioridad << "\n";
            actual = actual->siguiente;
        }
        file.close();
    }

    static NodoProcesso* cargarProcesos(const string& archivo) {
        ifstream file(archivo.c_str());
        if (!file.is_open()) {
            return NULL;
        }

        NodoProcesso* cabeza = NULL;
        NodoProcesso* ultimo = NULL;
        string linea;

        while (getline(file, linea)) {
            size_t pos1 = linea.find(',');
            size_t pos2 = linea.find(',', pos1+1);
            
            try {
                int id = atoi(linea.substr(0, pos1).c_str());
                string nombre = linea.substr(pos1+1, pos2-pos1-1);
                int prioridad = atoi(linea.substr(pos2+1).c_str());
                
                NodoProcesso* nuevo = new NodoProcesso(id, nombre, prioridad);
                if (!cabeza) {
                    cabeza = nuevo;
                } else {
                    ultimo->siguiente = nuevo;
                }
                ultimo = nuevo;
            } catch (...) {
                ErrorHandler::manejar(runtime_error("Formato inválido en línea: " + linea));
            }
        }
        file.close();
        return cabeza;
    }
};

/* ================================================================
 *                   GESTOR DE PROCESOS
 * ================================================================ */
class ListaProcesso {
private:
    NodoProcesso* cabeza;
    static const string ARCHIVO_PROCESOS; // Cambio para inicialización fuera de clase

public:
    ListaProcesso() : cabeza(NULL) {
        cabeza = Persistencia::cargarProcesos(ARCHIVO_PROCESOS);
    }

    ~ListaProcesso() {
        try {
            Persistencia::guardarProcesos(cabeza, ARCHIVO_PROCESOS);
        } catch (const exception& e) {
            ErrorHandler::manejar(e);
        }
        liberarMemoria();
    }

    void liberarMemoria() {
        while (cabeza) {
            NodoProcesso* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
        }
    }

    void insertarProcesso(int id, string nombre, int prioridad) {
        if (buscarPorId(id) != NULL) {
            throw runtime_error("ID " + to_string_alt(id) + " ya existe");
        }

        if (prioridad < 0 || prioridad > 100) {
            throw runtime_error("Prioridad debe ser 0-100");
        }

        NodoProcesso* nuevo = new NodoProcesso(id, nombre, prioridad);
        
        if (!cabeza) {
            cabeza = nuevo;
        } else {
            NodoProcesso* temp = cabeza;
            while (temp->siguiente) {
                temp = temp->siguiente;
            }
            temp->siguiente = nuevo;
        }
        cout << "Proceso insertado! (ID: " << id << ")\n";
    }

    void eliminarProcesso(int id) {
        if (!cabeza) {
            throw runtime_error("Lista vacía");
        }

        if (cabeza->id == id) {
            NodoProcesso* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
            cout << "Proceso eliminado!\n";
            return;
        }

        NodoProcesso* actual = cabeza;
        while (actual->siguiente && actual->siguiente->id != id) {
            actual = actual->siguiente;
        }

        if (!actual->siguiente) {
            throw runtime_error("Proceso no encontrado");
        }

        NodoProcesso* temp = actual->siguiente;
        actual->siguiente = temp->siguiente;
        delete temp;
        cout << "Proceso eliminado!\n";
    }

    NodoProcesso* buscarPorId(int id) const {
        NodoProcesso* temp = cabeza;
        while (temp) {
            if (temp->id == id) return temp;
            temp = temp->siguiente;
        }
        return NULL;
    }

    void mostrar() const {
        if (!cabeza) {
            cout << "\nNo hay procesos activos!\n";
            return;
        }
        
        NodoProcesso* temp = cabeza;
        cout << "\n--- Procesos Activos (" << contarProcesos() << ") ---\n";
        while (temp) {
            cout << "ID: " << temp->id 
                 << " | Nombre: " << temp->nombre 
                 << " | Prioridad: " << temp->prioridad << endl;
            temp = temp->siguiente;
        }
    }

    int contarProcesos() const {
        int count = 0;
        NodoProcesso* temp = cabeza;
        while (temp) {
            count++;
            temp = temp->siguiente;
        }
        return count;
    }
};

// Inicialización de miembro estático
const string ListaProcesso::ARCHIVO_PROCESOS = "procesos.dat";

/* ================================================================
 *                   PLANIFICADOR CPU
 * ================================================================ */
class ColaPrioridad {
private:
    struct NodoCola {
        NodoProcesso* proceso;
        NodoCola* siguiente;
        NodoCola(NodoProcesso* p) : proceso(p), siguiente(NULL) {}
    };
    
    NodoCola* frente;

public:
    ColaPrioridad() : frente(NULL) {}

    ~ColaPrioridad() {
        while (frente) {
            NodoCola* temp = frente;
            frente = frente->siguiente;
            delete temp;
        }
    }

    void encolarPrioridad(NodoProcesso* proceso) {
        if (!proceso) {
            throw runtime_error("Proceso inválido");
        }

        NodoCola* nuevo = new NodoCola(proceso);
        
        if (!frente || proceso->prioridad > frente->proceso->prioridad) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } else {
            NodoCola* actual = frente;
            while (actual->siguiente && 
                   actual->siguiente->proceso->prioridad >= proceso->prioridad) {
                actual = actual->siguiente;
            }
            nuevo->siguiente = actual->siguiente;
            actual->siguiente = nuevo;
        }
        cout << "Proceso encolado! (ID: " << proceso->id << ")\n";
    }

    NodoProcesso* desencolar() {
        if (!frente) {
            throw runtime_error("Cola vacía");
        }
        
        NodoCola* temp = frente;
        NodoProcesso* proceso = frente->proceso;
        frente = frente->siguiente;
        delete temp;
        
        cout << "Ejecutando proceso ID: " << proceso->id << endl;
        return proceso;
    }

    void mostrar() const {
        if (!frente) {
            cout << "\nCola de prioridad vacía!\n";
            return;
        }
        
        NodoCola* temp = frente;
        cout << "\n--- Cola de Prioridad (" << contarProcesos() << ") ---\n";
        while (temp) {
            cout << "ID: " << temp->proceso->id 
                 << " | Prioridad: " << temp->proceso->prioridad << endl;
            temp = temp->siguiente;
        }
    }

    int contarProcesos() const {
        int count = 0;
        NodoCola* temp = frente;
        while (temp) {
            count++;
            temp = temp->siguiente;
        }
        return count;
    }
};

/* ================================================================
 *                   GESTOR DE MEMORIA
 * ================================================================ */
class PilaMemoria {
private:
    struct NodoMemoria {
        int direccion;
        NodoMemoria* abajo;
        NodoMemoria(int dir) : direccion(dir), abajo(NULL) {}
    };
    
    NodoMemoria* tope;
    int capacidad;
    int contador;
    static const string ARCHIVO_MEMORIA;

public:
    PilaMemoria(int cap) : tope(NULL), capacidad(cap), contador(0) {}

    ~PilaMemoria() {
        while (tope) {
            NodoMemoria* temp = tope;
            tope = tope->abajo;
            delete temp;
        }
    }

    void push(int direccion) {
        if (contador >= capacidad) {
            throw runtime_error("Memoria llena");
        }
        
        NodoMemoria* nuevo = new NodoMemoria(direccion);
        nuevo->abajo = tope;
        tope = nuevo;
        contador++;
        
        cout << "Memoria asignada! (Dir: " << direccion << ")\n";
    }

    void pop() {
        if (!tope) {
            throw runtime_error("Memoria vacía");
        }
        
        NodoMemoria* temp = tope;
        tope = tope->abajo;
        cout << "Memoria liberada! (Dir: " << temp->direccion << ")\n";
        delete temp;
        contador--;
    }

    void estadoMemoria() const {
        cout << "\n--- Estado Memoria ---\n";
        cout << "Espacio usado: " << contador << "/" << capacidad << endl;
        
        if (tope) {
            cout << "Direcciones (tope primero): ";
            NodoMemoria* temp = tope;
            while (temp) {
                cout << temp->direccion << " ";
                temp = temp->abajo;
            }
            cout << endl;
        } else {
            cout << "No hay bloques asignados\n";
        }
    }
};

const string PilaMemoria::ARCHIVO_MEMORIA = "memoria.dat";

// ... (resto del código de interfaz permanece igual)

/* ================================================================
 *                   INTERFAZ DE USUARIO
 * ================================================================ */
int leerEntero(const string& mensaje, int min = INT_MIN, int max = INT_MAX) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor && valor >= min && valor <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Entrada inválida. Debe ser número entre " << min << " y " << max << endl;
    }
}

string leerCadena(const string& mensaje) {
    string valor;
    cout << mensaje;
    getline(cin >> ws, valor);
    return valor;
}

void mostrarMenuPrincipal() {
    system("clear || cls");
    cout << "\n=== SISTEMA OPERATIVO MINI v2.0 ===";
    cout << "\n1. Gestor de Procesos";
    cout << "\n2. Planificador CPU";
    cout << "\n3. Gestor de Memoria";
    cout << "\n4. Salir";
    cout << "\nSelección: ";
}

void menuProcesos(ListaProcesso& gestor) {
    int opcion;
    do {
        system("clear || cls");
        cout << "\n--- GESTOR DE PROCESOS ---";
        cout << "\n1. Insertar proceso";
        cout << "\n2. Eliminar proceso";
        cout << "\n3. Mostrar procesos";
        cout << "\n4. Volver";
        cout << "\nSelección: ";
        cin >> opcion;
        
        try {
            if (opcion == 1) {
                int id = leerEntero("ID: ", 0);
                string nombre = leerCadena("Nombre: ");
                int prioridad = leerEntero("Prioridad (0-100): ", 0, 100);
                gestor.insertarProcesso(id, nombre, prioridad);
            } else if (opcion == 2) {
                int id = leerEntero("ID a eliminar: ");
                gestor.eliminarProcesso(id);
            } else if (opcion == 3) {
                gestor.mostrar();
            }
        } catch (const exception& e) {
            ErrorHandler::manejar(e);
        }
        if (opcion != 4) system("pause");
    } while (opcion != 4);
}

void menuPlanificador(ListaProcesso& gestor, ColaPrioridad& planificador) {
    int opcion;
    do {
        system("clear || cls");
        cout << "\n--- PLANIFICADOR CPU ---";
        cout << "\n1. Encolar proceso";
        cout << "\n2. Ejecutar proceso";
        cout << "\n3. Mostrar cola";
        cout << "\n4. Volver";
        cout << "\nSelección: ";
        cin >> opcion;
        
        try {
            if (opcion == 1) {
                int id = leerEntero("ID del proceso: ");
                NodoProcesso* proc = gestor.buscarPorId(id);
                if (proc) {
                    planificador.encolarPrioridad(proc);
                } else {
                    cout << "Error: Proceso no encontrado!\n";
                }
            } else if (opcion == 2) {
                planificador.desencolar();
            } else if (opcion == 3) {
                planificador.mostrar();
            }
        } catch (const exception& e) {
            ErrorHandler::manejar(e);
        }
        if (opcion != 4) system("pause");
    } while (opcion != 4);
}

void menuMemoria(PilaMemoria& memoria) {
    int opcion;
    do {
        system("clear || cls");
        cout << "\n--- GESTOR DE MEMORIA ---";
        cout << "\n1. Asignar memoria";
        cout << "\n2. Liberar memoria";
        cout << "\n3. Estado memoria";
        cout << "\n4. Volver";
        cout << "\nSelección: ";
        cin >> opcion;
        
        try {
            if (opcion == 1) {
                int dir = leerEntero("Dirección: ");
                memoria.push(dir);
            } else if (opcion == 2) {
                memoria.pop();
            } else if (opcion == 3) {
                memoria.estadoMemoria();
            }
        } catch (const exception& e) {
            ErrorHandler::manejar(e);
        }
        if (opcion != 4) system("pause");
    } while (opcion != 4);
}

/* ================================================================
 *                   FUNCIÓN PRINCIPAL
 * ================================================================ */
int main() {
    ListaProcesso gestorProcesos;
    ColaPrioridad planificador;
    PilaMemoria memoria(3);

    int opcion;
    do {
        mostrarMenuPrincipal();
        try {
            opcion = leerEntero("", 1, 4);
            
            switch(opcion) {
                case 1:
                    menuProcesos(gestorProcesos);
                    break;
                case 2:
                    menuPlanificador(gestorProcesos, planificador);
                    break;
                case 3:
                    menuMemoria(memoria);
                    break;
                case 4:
                    cout << "Saliendo del sistema...\n";
                    break;
            }
        } catch (const exception& e) {
            ErrorHandler::manejar(e);
        }
    } while (opcion != 4);
    
    cout << "Sistema finalizado. Hasta pronto!\n";
    return 0;
}

