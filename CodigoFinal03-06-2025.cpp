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
/**
 * Implementación alternativa de to_string para compiladores antiguos
 * que no soportan std::to_string().
 * @tparam T Tipo del valor a convertir
 * @param value Valor a convertir a string
 * @return String que representa el valor
 */
template <typename T>
string to_string_alt(T value) {
    ostringstream os;
    os << value;
    return os.str();
}

/* ================================================================
 *                   GESTOR DE ERRORES
 * ================================================================ */
/**
 * Clase para manejo centralizado de errores.
 * Proporciona funcionalidad para mostrar errores en consola
 * y registrarlos en un archivo de log.
 */
class ErrorHandler {
public:
    /**
     * Maneja una excepción mostrándola en consola y guardándola en log.
     * @param e Excepción a manejar
     */
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
/**
 * Clase que representa un nodo de proceso en la lista enlazada.
 * Contiene información básica del proceso y un puntero al siguiente nodo.
 */
class NodoProcesso {
public:
    int id;             // Identificador único del proceso
    string nombre;      // Nombre descriptivo del proceso
    int prioridad;      // Prioridad del proceso (0-100)
    NodoProcesso* siguiente; // Puntero al siguiente nodo en la lista

    /**
     * Constructor del nodo de proceso.
     * @param id Identificador del proceso
     * @param nombre Nombre del proceso
     * @param prioridad Nivel de prioridad (0-100)
     */
    NodoProcesso(int id, string nombre, int prioridad) 
        : id(id), nombre(nombre), prioridad(prioridad), siguiente(NULL) {}
};

/* ================================================================
 *                   GESTOR DE PERSISTENCIA
 * ================================================================ */
/**
 * Clase para manejar la persistencia de datos en archivos.
 * Proporciona métodos para guardar y cargar procesos desde/hacia archivos.
 */
class Persistencia {
public:
    /**
     * Guarda la lista de procesos en un archivo.
     * @param cabeza Puntero al primer nodo de la lista
     * @param archivo Nombre del archivo donde guardar
     * @throws runtime_error Si no se puede abrir el archivo
     */
    static void guardarProcesos(NodoProcesso* cabeza, const string& archivo) {
        ofstream file(archivo.c_str());
        if (!file.is_open()) {
            throw runtime_error("No se pudo abrir " + archivo + " para escritura");
        }
        
        // Recorre la lista y escribe cada proceso en el archivo
        NodoProcesso* actual = cabeza;
        while (actual) {
            file << actual->id << "," << actual->nombre << "," << actual->prioridad << "\n";
            actual = actual->siguiente;
        }
        file.close();
    }

    /**
     * Carga procesos desde un archivo a una lista enlazada.
     * @param archivo Nombre del archivo a cargar
     * @return Puntero al primer nodo de la lista cargada (NULL si no existe archivo)
     */
    static NodoProcesso* cargarProcesos(const string& archivo) {
        ifstream file(archivo.c_str());
        if (!file.is_open()) {
            return NULL;
        }

        NodoProcesso* cabeza = NULL;
        NodoProcesso* ultimo = NULL;
        string linea;

        // Lee cada línea del archivo y crea los nodos correspondientes
        while (getline(file, linea)) {
            size_t pos1 = linea.find(',');
            size_t pos2 = linea.find(',', pos1+1);
            
            try {
                // Extrae los datos de cada línea
                int id = atoi(linea.substr(0, pos1).c_str());
                string nombre = linea.substr(pos1+1, pos2-pos1-1);
                int prioridad = atoi(linea.substr(pos2+1).c_str());
                
                // Crea un nuevo nodo y lo añade a la lista
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
/**
 * Clase que gestiona una lista enlazada de procesos.
 * Proporciona operaciones CRUD para los procesos y maneja su persistencia.
 */
class ListaProcesso {
private:
    NodoProcesso* cabeza; // Puntero al primer nodo de la lista
    static const string ARCHIVO_PROCESOS; // Nombre del archivo de persistencia

public:
    /**
     * Constructor que carga los procesos desde archivo al iniciar.
     */
    ListaProcesso() : cabeza(NULL) {
        cabeza = Persistencia::cargarProcesos(ARCHIVO_PROCESOS);
    }

    /**
     * Destructor que guarda los procesos al archivo y libera memoria.
     */
    ~ListaProcesso() {
        try {
            Persistencia::guardarProcesos(cabeza, ARCHIVO_PROCESOS);
        } catch (const exception& e) {
            ErrorHandler::manejar(e);
        }
        liberarMemoria();
    }

    /**
     * Libera toda la memoria ocupada por la lista de procesos.
     */
    void liberarMemoria() {
        while (cabeza) {
            NodoProcesso* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
        }
    }

    /**
     * Inserta un nuevo proceso en la lista.
     * @param id Identificador del proceso (debe ser único)
     * @param nombre Nombre del proceso
     * @param prioridad Nivel de prioridad (0-100)
     * @throws runtime_error Si el ID ya existe o la prioridad es inválida
     */
    void insertarProcesso(int id, string nombre, int prioridad) {
        if (buscarPorId(id) != NULL) {
            throw runtime_error("ID " + to_string_alt(id) + " ya existe");
        }

        if (prioridad < 0 || prioridad > 100) {
            throw runtime_error("Prioridad debe ser 0-100");
        }

        NodoProcesso* nuevo = new NodoProcesso(id, nombre, prioridad);
        
        // Inserta al final de la lista
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

    /**
     * Elimina un proceso de la lista por su ID.
     * @param id Identificador del proceso a eliminar
     * @throws runtime_error Si la lista está vacía o el ID no existe
     */
    void eliminarProcesso(int id) {
        if (!cabeza) {
            throw runtime_error("Lista vacía");
        }

        // Caso especial: eliminar el primer nodo
        if (cabeza->id == id) {
            NodoProcesso* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
            cout << "Proceso eliminado!\n";
            return;
        }

        // Busca el nodo anterior al que se quiere eliminar
        NodoProcesso* actual = cabeza;
        while (actual->siguiente && actual->siguiente->id != id) {
            actual = actual->siguiente;
        }

        if (!actual->siguiente) {
            throw runtime_error("Proceso no encontrado");
        }

        // Elimina el nodo y ajusta los punteros
        NodoProcesso* temp = actual->siguiente;
        actual->siguiente = temp->siguiente;
        delete temp;
        cout << "Proceso eliminado!\n";
    }

    /**
     * Busca un proceso por su ID.
     * @param id Identificador a buscar
     * @return Puntero al nodo encontrado o NULL si no existe
     */
    NodoProcesso* buscarPorId(int id) const {
        NodoProcesso* temp = cabeza;
        while (temp) {
            if (temp->id == id) return temp;
            temp = temp->siguiente;
        }
        return NULL;
    }

    /**
     * Muestra todos los procesos en la lista.
     */
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

    /**
     * Cuenta la cantidad de procesos en la lista.
     * @return Número de procesos
     */
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
/**
 * Clase que implementa una cola de prioridad para planificación de procesos.
 * Los procesos con mayor prioridad se ejecutan primero.
 */
class ColaPrioridad {
private:
    // Nodo interno para la cola de prioridad
    struct NodoCola {
        NodoProcesso* proceso;  // Puntero al proceso
        NodoCola* siguiente;    // Puntero al siguiente nodo en la cola
        NodoCola(NodoProcesso* p) : proceso(p), siguiente(NULL) {}
    };
    
    NodoCola* frente; // Puntero al frente de la cola

public:
    /**
     * Constructor que inicializa una cola vacía.
     */
    ColaPrioridad() : frente(NULL) {}

    /**
     * Destructor que libera la memoria de los nodos de la cola.
     */
    ~ColaPrioridad() {
        while (frente) {
            NodoCola* temp = frente;
            frente = frente->siguiente;
            delete temp;
        }
    }

    /**
     * Añade un proceso a la cola según su prioridad.
     * @param proceso Puntero al proceso a encolar
     * @throws runtime_error Si el proceso es inválido
     */
    void encolarPrioridad(NodoProcesso* proceso) {
        if (!proceso) {
            throw runtime_error("Proceso inválido");
        }

        NodoCola* nuevo = new NodoCola(proceso);
        
        // Inserta al frente si la cola está vacía o tiene mayor prioridad
        if (!frente || proceso->prioridad > frente->proceso->prioridad) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } else {
            // Busca la posición correcta según prioridad
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

    /**
     * Extrae y devuelve el proceso con mayor prioridad.
     * @return Puntero al proceso a ejecutar
     * @throws runtime_error Si la cola está vacía
     */
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

    /**
     * Muestra los procesos en la cola de prioridad.
     */
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

    /**
     * Cuenta los procesos en la cola.
     * @return Número de procesos en cola
     */
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
/**
 * Clase que implementa una pila para gestión de memoria.
 * Simula asignación y liberación de bloques de memoria.
 */
class PilaMemoria {
private:
    // Nodo interno para la pila de memoria
    struct NodoMemoria {
        int direccion;         // Dirección de memoria
        NodoMemoria* abajo;    // Puntero al nodo inferior en la pila
        NodoMemoria(int dir) : direccion(dir), abajo(NULL) {}
    };
    
    NodoMemoria* tope;     // Puntero al tope de la pila
    int capacidad;          // Capacidad máxima de la pila
    int contador;           // Contador de bloques asignados
    static const string ARCHIVO_MEMORIA; // Archivo para persistencia

public:
    /**
     * Constructor que inicializa la pila de memoria.
     * @param cap Capacidad máxima de la pila
     */
    PilaMemoria(int cap) : tope(NULL), capacidad(cap), contador(0) {}

    /**
     * Destructor que libera todos los nodos de la pila.
     */
    ~PilaMemoria() {
        while (tope) {
            NodoMemoria* temp = tope;
            tope = tope->abajo;
            delete temp;
        }
    }

    /**
     * Asigna un nuevo bloque de memoria (push en la pila).
     * @param direccion Dirección de memoria a asignar
     * @throws runtime_error Si la memoria está llena
     */
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

    /**
     * Libera el último bloque de memoria asignado (pop de la pila).
     * @throws runtime_error Si la memoria está vacía
     */
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

    /**
     * Muestra el estado actual de la memoria.
     */
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

/* ================================================================
 *                   INTERFAZ DE USUARIO
 * ================================================================ */
/**
 * Lee un entero de la entrada estándar con validación.
 * @param mensaje Mensaje a mostrar al usuario
 * @param min Valor mínimo permitido
 * @param max Valor máximo permitido
 * @return Entero válido introducido por el usuario
 */
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

/**
 * Lee una cadena de la entrada estándar.
 * @param mensaje Mensaje a mostrar al usuario
 * @return Cadena introducida por el usuario
 */
string leerCadena(const string& mensaje) {
    string valor;
    cout << mensaje;
    getline(cin >> ws, valor);
    return valor;
}

/**
 * Muestra el menú principal del sistema.
 */
void mostrarMenuPrincipal() {
    system("clear || cls");
    cout << "\n=== SISTEMA OPERATIVO MINI v2.0 ===";
    cout << "\n1. Gestor de Procesos";
    cout << "\n2. Planificador CPU";
    cout << "\n3. Gestor de Memoria";
    cout << "\n4. Salir";
    cout << "\nSelección: ";
}

/**
 * Muestra el menú de gestión de procesos.
 * @param gestor Referencia al gestor de procesos
 */
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

/**
 * Muestra el menú del planificador de CPU.
 * @param gestor Referencia al gestor de procesos
 * @param planificador Referencia al planificador de CPU
 */
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

/**
 * Muestra el menú de gestión de memoria.
 * @param memoria Referencia al gestor de memoria
 */
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
    // Inicialización de los componentes principales
    ListaProcesso gestorProcesos;  // Gestor de procesos
    ColaPrioridad planificador;    // Planificador de CPU
    PilaMemoria memoria(3);        // Gestor de memoria con capacidad 3

    int opcion;
    do {
        mostrarMenuPrincipal();
        try {
            opcion = leerEntero("", 1, 4);
            
            // Manejo de las opciones del menú principal
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
