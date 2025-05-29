//Huaman Brañez Jose Antonio
#include <iostream>
#include <string>
using namespace std;

/* ================================================================
 *                   GESTOR DE PROCESOS
 * (Lista Enlazada Simple para almacenar información de procesos)
 * ================================================================ */

// Nodo para almacenar información de un proceso
class NodoProcesso {
public:
    int id;                  // Identificador único del proceso
    string nombre;           // Nombre del proceso
    int prioridad;           // Prioridad (0 = baja, 100 = alta)
    NodoProcesso* siguiente; // Puntero al siguiente nodo en la lista

    // Constructor para inicializar un nuevo proceso
    NodoProcesso(int id, string nombre, int prioridad) 
        : id(id), nombre(nombre), prioridad(prioridad), siguiente(NULL) {}
};

// Lista enlazada para gestionar todos los procesos del sistema
class ListaProcesso {
private:
    NodoProcesso* cabeza;    // Puntero al primer nodo de la lista

public:
    // Constructor: inicializa lista vacía
    ListaProcesso() : cabeza(NULL) {}

    // Insertar nuevo proceso al final de la lista
    void insertarProcesso(int id, string nombre, int prioridad) {
        // Crear nuevo nodo
        NodoProcesso* nuevo = new NodoProcesso(id, nombre, prioridad);
        
        // Si la lista está vacía
        if (!cabeza) {
            cabeza = nuevo;
        } 
        // Si ya hay elementos, buscar el último
        else {
            NodoProcesso* temp = cabeza;
            while (temp->siguiente) {
                temp = temp->siguiente;
            }
            temp->siguiente = nuevo;
        }
        cout << "Proceso insertado! (ID: " << id << ")\n";
    }

    // Eliminar proceso por ID
    void eliminarProcesso(int id) {
        // Si la lista está vacía
        if (!cabeza) {
            cout << "Lista vacia!\n";
            return;
        }

        // Caso especial: eliminar el primer elemento
        if (cabeza->id == id) {
            NodoProcesso* temp = cabeza;
            cabeza = cabeza->siguiente;
            delete temp;
            cout << "Proceso eliminado!\n";
            return;
        }

        // Buscar el proceso a eliminar
        NodoProcesso* actual = cabeza;
        while (actual->siguiente && actual->siguiente->id != id) {
            actual = actual->siguiente;
        }

        // Si encontró el proceso
        if (actual->siguiente) {
            NodoProcesso* temp = actual->siguiente;
            actual->siguiente = temp->siguiente;
            delete temp;
            cout << "Proceso eliminado!\n";
        } else {
            cout << "Proceso no encontrado!\n";
        }
    }

    // Buscar proceso por ID (devuelve puntero o NULL)
    NodoProcesso* buscarPorId(int id) {
        NodoProcesso* temp = cabeza;
        while (temp) {
            if (temp->id == id) return temp;
            temp = temp->siguiente;
        }
        return NULL;
    }

    // Mostrar todos los procesos en la lista
    void mostrar() {
        // Si la lista está vacía
        if (!cabeza) {
            cout << "\nNo hay procesos activos!\n";
            return;
        }
        
        // Recorrer lista e imprimir cada proceso
        NodoProcesso* temp = cabeza;
        cout << "\n--- Procesos Activos ---\n";
        while (temp) {
            cout << "ID: " << temp->id 
                 << " | Nombre: " << temp->nombre 
                 << " | Prioridad: " << temp->prioridad << endl;
            temp = temp->siguiente;
        }
    }
};

/* ================================================================
 *                   PLANIFICADOR CPU
 * (Cola de Prioridad para gestionar ejecución de procesos)
 * ================================================================ */

class ColaPrioridad {
private:
    // Nodo interno para la cola de prioridad
    struct NodoCola {
        NodoProcesso* proceso; // Puntero al proceso en la lista principal
        NodoCola* siguiente;   // Puntero al siguiente nodo en la cola
        
        NodoCola(NodoProcesso* p) : proceso(p), siguiente(NULL) {}
    };
    
    NodoCola* frente; // Proceso con mayor prioridad

public:
    // Constructor: cola vacía
    ColaPrioridad() : frente(NULL) {}

    // Añadir proceso a la cola ordenado por prioridad
    void encolarPrioridad(NodoProcesso* proceso) {
        // Crear nuevo nodo para la cola
        NodoCola* nuevo = new NodoCola(proceso);
        
        // Caso 1: Cola vacía o nueva prioridad mayor que el frente
        if (!frente || proceso->prioridad > frente->proceso->prioridad) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } 
        // Caso 2: Buscar posición adecuada en la cola
        else {
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

    // Sacar el proceso de mayor prioridad para ejecución
    NodoProcesso* desencolar() {
        // Si la cola está vacía
        if (!frente) {
            cout << "Cola vacia!\n";
            return NULL;
        }
        
        // Sacar el primer proceso
        NodoCola* temp = frente;
        NodoProcesso* proceso = frente->proceso;
        frente = frente->siguiente;
        delete temp;
        
        cout << "Ejecutando proceso ID: " << proceso->id << endl;
        return proceso;
    }

    // Mostrar todos los procesos en la cola
    void mostrar() {
        // Si la cola está vacía
        if (!frente) {
            cout << "\nCola de prioridad vacia!\n";
            return;
        }
        
        // Recorrer cola e imprimir procesos
        NodoCola* temp = frente;
        cout << "\n--- Cola de Prioridad ---\n";
        while (temp) {
            cout << "ID: " << temp->proceso->id 
                 << " | Prioridad: " << temp->proceso->prioridad << endl;
            temp = temp->siguiente;
        }
    }
};

/* ================================================================
 *                   GESTOR DE MEMORIA
 * (Pila LIFO para simular asignación de memoria)
 * ================================================================ */

class PilaMemoria {
private:
    // Nodo para almacenar direcciones de memoria
    struct NodoMemoria {
        int direccion;        // Dirección de memoria
        NodoMemoria* abajo;   // Puntero al nodo inferior en la pila
        
        NodoMemoria(int dir) : direccion(dir), abajo(NULL) {}
    };
    
    NodoMemoria* tope;    // Último bloque de memoria asignado
    int capacidad;        // Máximo de bloques de memoria disponibles
    int contador;         // Bloques actualmente en uso

public:
    // Constructor: inicializa pila vacía
    PilaMemoria(int cap) : tope(NULL), capacidad(cap), contador(0) {}

    // Asignar nuevo bloque de memoria (push)
    void push(int direccion) {
        // Verificar si hay espacio disponible
        if (contador >= capacidad) {
            cout << "Memoria llena! No se puede asignar.\n";
            return;
        }
        
        // Crear nuevo nodo y colocarlo en el tope
        NodoMemoria* nuevo = new NodoMemoria(direccion);
        nuevo->abajo = tope;
        tope = nuevo;
        contador++;
        
        cout << "Memoria asignada! (Dir: " << direccion << ")\n";
    }

    // Liberar el último bloque asignado (pop)
    void pop() {
        // Verificar si hay bloques asignados
        if (!tope) {
            cout << "Memoria vacia! No hay nada que liberar.\n";
            return;
        }
        
        // Eliminar el bloque del tope
        NodoMemoria* temp = tope;
        tope = tope->abajo;
        cout << "Memoria liberada! (Dir: " << temp->direccion << ")\n";
        delete temp;
        contador--;
    }

    // Mostrar estado actual de la memoria
    void estadoMemoria() {
        cout << "\n--- Estado Memoria ---\n";
        cout << "Espacio usado: " << contador << "/" << capacidad << endl;
        
        // Si hay bloques asignados, mostrar direcciones
        if (tope) {
            cout << "Direcciones (tope primero): ";
            NodoMemoria* temp = tope;
            while (temp) {
                cout << temp->direccion << " ";
                temp = temp->abajo;
            }
            cout << endl;
        } else {
            cout << "No hay bloques de memoria asignados.\n";
        }
    }
};

/* ================================================================
 *                   INTERFAZ DE USUARIO
 * (Menús para interactuar con el sistema)
 * ================================================================ */

// Función para mostrar el menú principal
void mostrarMenuPrincipal() {
    cout << "\n=== SISTEMA OPERATIVO MINI ===";
    cout << "\n1. Gestor de Procesos";
    cout << "\n2. Planificador CPU";
    cout << "\n3. Gestor de Memoria";
    cout << "\n4. Salir";
    cout << "\nSeleccion: ";
}

// Función para mostrar el menú de Gestor de Procesos
void menuProcesos(ListaProcesso& gestor) {
    int opcion;
    do {
        cout << "\n--- GESTOR DE PROCESOS ---";
        cout << "\n1. Insertar proceso";
        cout << "\n2. Eliminar proceso";
        cout << "\n3. Mostrar procesos";
        cout << "\n4. Volver";
        cout << "\nSeleccion: ";
        cin >> opcion;
        
        if (opcion == 1) {
            int id, prioridad;
            string nombre;
            cout << "ID: "; cin >> id;
            cout << "Nombre: "; cin >> nombre;
            cout << "Prioridad: "; cin >> prioridad;
            gestor.insertarProcesso(id, nombre, prioridad);
        } else if (opcion == 2) {
            int id;
            cout << "ID a eliminar: "; cin >> id;
            gestor.eliminarProcesso(id);
        } else if (opcion == 3) {
            gestor.mostrar();
        }
    } while (opcion != 4);
}

// Función para mostrar el menú de Planificador CPU
void menuPlanificador(ListaProcesso& gestor, ColaPrioridad& planificador) {
    int opcion;
    do {
        cout << "\n--- PLANIFICADOR CPU ---";
        cout << "\n1. Encolar proceso";
        cout << "\n2. Ejecutar proceso";
        cout << "\n3. Mostrar cola";
        cout << "\n4. Volver";
        cout << "\nSeleccion: ";
        cin >> opcion;
        
        if (opcion == 1) {
            int id;
            cout << "ID del proceso: "; cin >> id;
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
    } while (opcion != 4);
}

// Función para mostrar el menú de Gestor de Memoria
void menuMemoria(PilaMemoria& memoria) {
    int opcion;
    do {
        cout << "\n--- GESTOR DE MEMORIA ---";
        cout << "\n1. Asignar memoria";
        cout << "\n2. Liberar memoria";
        cout << "\n3. Estado memoria";
        cout << "\n4. Volver";
        cout << "\nSeleccion: ";
        cin >> opcion;
        
        if (opcion == 1) {
            int dir;
            cout << "Direccion: "; cin >> dir;
            memoria.push(dir);
        } else if (opcion == 2) {
            memoria.pop();
        } else if (opcion == 3) {
            memoria.estadoMemoria();
        }
    } while (opcion != 4);
}

/* ================================================================
 *                   FUNCIÓN PRINCIPAL
 * (Punto de entrada del programa)
 * ================================================================ */

int main() {
    // Crear instancias de los componentes del sistema
    ListaProcesso gestorProcesos;
    ColaPrioridad planificador;
    PilaMemoria memoria(3);  // Memoria limitada a 3 bloques
    
    // Menú principal
    int opcion;
    do {
        mostrarMenuPrincipal();
        cin >> opcion;
        
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
            default:
                cout << "Opcion invalida! Intente de nuevo.\n";
        }
    } while (opcion != 4);
    
    cout << "Sistema finalizado. Hasta pronto!\n";
    return 0;
}

