#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <regex>
#include <iomanip>
#include <chrono>

using namespace std;

#pragma pack(push, 1)
struct Persona { // Total 162 bytes
    uint32_t dni; //4 bytes            
    char nombres[20];          
    char apellidos[20];
    char departamento[15];     
    char provincia[15];        
    char ciudad[15];           
    char distrito[15];         
    char ubicacion[15];        
    char telefono[10];         
    char email[20];            
    char estado_civil[12];      
    bool activo; //1 byte           
}; 
#pragma pack(pop)

//struct que se almacenaran en cada slot de las tablas, para luego guardarlas en el indice
#pragma pack(push, 1)
struct ElementoIndice {
    uint32_t dni; 
    uint64_t offset;
    bool ocupado; 

    ElementoIndice() : dni(0), offset(0), ocupado(false) {}
};
#pragma pack(pop)

//cabezera del indice
#pragma pack(push, 1)
struct HeaderIndice {
    uint32_t num_tablas;
    uint32_t tamaño_tabla;
    uint32_t num_elementos;
};
#pragma pack(pop)

//sistema de gestion de millones de registros de personas
class SistemaCuckooHash {
private:
    static constexpr uint32_t NUM_TABLAS = 4;
    static constexpr uint32_t TAMAÑO_TABLA = 20000000;
    //se inicializan con constexpr para tener 1 sola copia en memoria para todas las instancias
    int MAX_INTENTOS = 500;  // No constante para poder duplicar
    
    vector<vector<ElementoIndice>> tablas; //declaramos las tablas que almacenaran structs
    uint32_t numElementos; //total de elementos insertados
    fstream archivoData;
    string rutaData;
    string rutaIndice;

    //funciones hash
    size_t hash1(uint32_t dni) const {
        return (dni / 7919 + dni * 13) % TAMAÑO_TABLA;
    }

    size_t hash2(uint32_t dni) const {
        return ((dni >> 12) + dni * 17) % TAMAÑO_TABLA;
    }

    size_t hash3(uint32_t dni) const {
        return (dni * 23 + (dni % 7919)) % TAMAÑO_TABLA;
    }

    size_t hash4(uint32_t dni) const {
        return ((dni + 1234567) * 7) % TAMAÑO_TABLA;
    }
    
    size_t hash_function(uint32_t dni, int tabla_num) const {
        switch(tabla_num) {
            case 0: return hash1(dni);
            case 1: return hash2(dni);
            case 2: return hash3(dni);
            case 3: return hash4(dni);
            default: return hash1(dni);
        }
    }

    //validaciones
    bool validarDNI(uint32_t dni) {
        return dni >= 10000000 && dni <= 99999999;
    }
    //solo letras y menos del tamaño maximo de caracteres para segun el struct Persona
    bool validarCampoTexto(const string& texto, size_t maximo) {
        if (texto.empty() || texto.length() >= maximo){
            cout<<"Error: el campo no puede estar vacio y solo se permite maximo "<<(maximo-1)<<" caracteres\n";
            return false;
        } 
        for (char c : texto) {
            if (!isalpha(c)&& c != ' '){
                cout<<"Error: solo se permiten letras y espacios\n";
                return false;
            } 
        }
        return true;
    }
    //formato [algo]@gmail.com
    bool validarEmail(const string& email) {
        regex patron(R"(^[a-zA-Z0-9]+@g\.com$)");
        return regex_match(email, patron) && email.length() < 20;
    }
    //debe empezar con 9 y tener 9 digitos exactos
    bool validarTelefono(const string& telefono) {        
        if (telefono.length() != 9) return false;
        if (telefono[0] != '9') return false;
        
        for (char c : telefono) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

    //validar que este en el rango 1-4
    bool validarEstadoCivil(int opcion) {
        if (opcion >= 1 && opcion <= 4) {
            return true;
        }  
            return false;  
    }

    //busqueda en cuckoo
        //codigo
    
    //funcionalidad interna de insercion con cuckoo
    bool insertarEnCuckoo(const ElementoIndice& nuevo_indice) {
        static int tabla_inicial = 0;
        int tabla_actual = tabla_inicial;
        tabla_inicial = (tabla_inicial + 1) % NUM_TABLAS;

        ElementoIndice actual = nuevo_indice;

        for (int intento = 0; intento < MAX_INTENTOS; intento++) {
            size_t pos = hash_function(actual.dni, tabla_actual);
            
            if (!tablas[tabla_actual][pos].ocupado) {
                tablas[tabla_actual][pos] = actual;
                numElementos++;
                return true;
            }

            for(int offset = 1; offset < NUM_TABLAS; offset++){
                int tabla_siguiente = (tabla_actual+offset)%NUM_TABLAS;
                size_t pos_sig = hash_function(actual.dni, tabla_siguiente);

                if (!tablas[tabla_siguiente][pos_sig].ocupado) {
                    tablas[tabla_siguiente][pos_sig] = actual;
                    numElementos++;
                    return true;
                }
            }
            
            size_t pos_actual = hash_function(actual.dni, tabla_actual);
            swap(actual, tablas[tabla_actual][pos_actual]);
            
            tabla_actual = (tabla_actual + 1) % NUM_TABLAS;
        }
        return false;
    }

    public:
    //constructor
    SistemaCuckooHash(const string& ruta_data, const string& ruta_indice) 
        : numElementos(0), rutaData(ruta_data), rutaIndice(ruta_indice) {
        //inicializamos las 4 tablas con su respectivo tamaño cada una
        tablas.resize(NUM_TABLAS);
        for (int i = 0; i < NUM_TABLAS; i++) {
            tablas[i].resize(TAMAÑO_TABLA);
        }
    }
    //destructor para cerrar el archivo de indice
    ~SistemaCuckooHash() {
        if (archivoData.is_open()) {
            archivoData.close();
        }
    }

    //funcion para cargar indice
        //codigo

    //funcion para guardar indice
        //codigo

    //funciona para abirr dataset
        //codigo


    //funcion para validar factor de carga
        //codigo

    //funcion para buscar con dni
        //codigo


    //funcionalidad para insertar nuevo registro de persona
    void insertar() {
        cout << "\n     INSERTAR NUEVA PERSONA\n";
        
        Persona nueva;
        string temp;
        
        cout << "DNI (8 digitos): ";
        cin >> nueva.dni;
        cin.ignore();
        
        if (!validarDNI(nueva.dni)) {
            cout << " ERROR: DNI invalido\n";
            return;
        }
        
        //verificar si ya existe
        int tabla;
        size_t posicion;
        if (buscarEnIndice(nueva.dni, tabla, posicion)) {
            cout << " ERROR: El DNI ya existe en el sistema\n";
            cout << "No se permite registrar el mismo DNI dos veces\n";
            return;
        }
        
        //nombres
        cout << "Nombres: ";
        getline(cin, temp);
        
        if (!validarCampoTexto(temp,sizeof(nueva.nombres))) {
            cout << " ERROR: Nombres invalidos\n";
            return;
        }
        strncpy(nueva.nombres, temp.c_str(),19);
        nueva.nombres[19] = '\0';

        //apellidos
        cout << "Apellidos: ";
        getline(cin, temp);
        
        if (!validarCampoTexto(temp,sizeof(nueva.apellidos))) {
            cout << " ERROR: Apellidos invalidos\n";
            return;
        }
        strncpy(nueva.apellidos, temp.c_str(),19);
        nueva.apellidos[19] = '\0';

        //departamento
        cout << "Departamento: ";
        getline(cin, temp);
        if (!validarCampoTexto(temp,sizeof(nueva.departamento))) {
            cout << " ERROR: Departamento invalido\n";
            return;
        }
        strncpy(nueva.departamento, temp.c_str(),14);
        nueva.departamento[14] = '\0';

        //provincia
        cout << "Provincia: ";
        getline(cin, temp);
        if (!validarCampoTexto(temp,sizeof(nueva.provincia))) {
            cout << " ERROR: Provincia invalida\n";
            return;
        }
        strncpy(nueva.provincia, temp.c_str(), 14);
        nueva.provincia[14] = '\0';
        
        //ciudad
        cout << "Ciudad: ";
        getline(cin, temp);
        if (!validarCampoTexto(temp,sizeof(nueva.ciudad))) {
            cout << " ERROR: Ciudad invalida\n";
            return;
        }
        strncpy(nueva.ciudad, temp.c_str(), 14);
        nueva.ciudad[14] = '\0';
        
        //distrito
        cout << "Distrito: ";
        getline(cin, temp);
        if (!validarCampoTexto(temp,sizeof(nueva.distrito))) {
            cout << " ERROR: Distrito invalido\n";
            return;
        }
        strncpy(nueva.distrito, temp.c_str(), 14);
        nueva.distrito[14] = '\0';
        
        //ubicación
        cout << "Ubicacion (direccion): ";
        getline(cin, temp);
        if (!validarCampoTexto(temp,sizeof(nueva.ubicacion))) {
            cout << " ERROR: Ubicacion invalida\n";
            return;
        }
        strncpy(nueva.ubicacion, temp.c_str(), 14);
        nueva.ubicacion[14] = '\0';
        
        //telefono
        cout << "Telefono (9 digitos, empezando con 9): ";
        getline(cin, temp);
        
        if (!validarTelefono(temp)) {
            cout << " ERROR: Telefono invalido\n";
            return;
        }
        strncpy(nueva.telefono, temp.c_str(), 9);
        nueva.telefono[9] = '\0';
        
        //email
        cout << "Email ([texto-numeros]@g.com): ";
        getline(cin, temp);
        
        if (!validarEmail(temp)) {
            cout << " ERROR: Email invalido. Debe tener maximo 19 caracteres y un formato [texto-numeros]@g.com\n";
            return;
        }
        strncpy(nueva.email, temp.c_str(), 19);
        nueva.email[19] = '\0';
        
        //estado civil
        cout << "Estado Civil:\n";
        cout << "1. Soltero\n";
        cout << "2. Casado\n";
        cout << "3. Divorciado\n";
        cout << "4. Viudo\n";
        cout << "Seleccione (1-4): ";
        int opcion;
        cin >> opcion;
        cin.ignore();

        if (!validarEstadoCivil(opcion)) {
            cout << " ERROR: Debe seleccionar una opcion entre 1 y 4\n";
            return;
        }
        switch (opcion) {
            case 1: strcpy(nueva.estado_civil, "Soltero"); break;
            case 2: strcpy(nueva.estado_civil, "Casado"); break;
            case 3: strcpy(nueva.estado_civil, "Divorciado"); break;
            case 4: strcpy(nueva.estado_civil, "Viudo"); break;
        }
        
        //activo por defecto
        nueva.activo = true;
        
        //escribe en archivo de datos
        archivoData.seekp(0, ios::end);
        uint64_t offset = archivoData.tellp();
        archivoData.write(reinterpret_cast<const char*>(&nueva), sizeof(Persona));
        archivoData.flush();
        
        //inserta en andice
        ElementoIndice nuevoIndice;
        nuevoIndice.dni = nueva.dni;
        nuevoIndice.offset = offset;
        nuevoIndice.ocupado = true;
        
        if (insertarEnCuckoo(nuevoIndice)) {
            cout << "\n Persona insertada exitosamente\n";
            return;
        }
        
        //si falla se duplica el MAX_INTENTOS y reintenta insercion
        cout << "\nADVERTENCIA: Primera insercion fallo. Duplicando intentos\n";
        MAX_INTENTOS = 1000;
        
        if (insertarEnCuckoo(nuevoIndice)){
            cout << " Persona insertada exitosamente (con intentos extendidos)\n";
            MAX_INTENTOS = 500; 
            return;
        }
        
        //si aun falla mostrar error
        cout << " ERROR: No se pudo insertar la persona\n";    
        MAX_INTENTOS = 500; //se restaura
    } 

    //funcion para eliminar con dni
        //codigo

    //funcion de mostrar menu
        //codigo

}
