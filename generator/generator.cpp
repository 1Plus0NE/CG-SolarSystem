#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <cmath>

using namespace std;

// Gera 2 triângulos para formar um quadrado
// p1 --- p2
// |  \   |
// |   \  |
// p3 --- p4
void generateSquare(list<string>& vertices, 
                    float x1, float y1, float z1,  // p1
                    float x2, float y2, float z2,  // p2
                    float x3, float y3, float z3,  // p3
                    float x4, float y4, float z4) { // p4
    // Triângulo 1: p1, p2, p4
    vertices.push_back(to_string(x1) + " " + to_string(y1) + " " + to_string(z1));
    vertices.push_back(to_string(x2) + " " + to_string(y2) + " " + to_string(z2));
    vertices.push_back(to_string(x4) + " " + to_string(y4) + " " + to_string(z4));
    
    // Triângulo 2: p1, p4, p3
    vertices.push_back(to_string(x1) + " " + to_string(y1) + " " + to_string(z1));
    vertices.push_back(to_string(x4) + " " + to_string(y4) + " " + to_string(z4));
    vertices.push_back(to_string(x3) + " " + to_string(y3) + " " + to_string(z3));
}

void generateBox(float length, int divisions, list<string>& vertices) {
    float half = length / 2.0f;  // Para centrar na origem
    float step = length / divisions; // Distância entre vértices

    // Gerar as 6 faces do cubo em ordem counter-clockwise (CCW)
    // Quando vistas de FORA do cubo, os vértices devem estar em sentido anti-horário

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step;
            float x1 = -half + (i + 1) * step;
            float y0 = -half + j * step;
            float y1 = -half + (j + 1) * step;
            
            // Face frontal (z = +half) - olhando de +Z para origem
            // CCW quando visto de fora: baixo-esq, baixo-dir, cima-esq, cima-dir
            generateSquare(vertices,
                x0, y0, half,   // p1: baixo-esquerda
                x1, y0, half,   // p2: baixo-direita
                x0, y1, half,   // p3: cima-esquerda
                x1, y1, half);  // p4: cima-direita
            
            // Face traseira (z = -half) - olhando de -Z para origem
            // CCW quando visto de fora: baixo-dir, baixo-esq, cima-dir, cima-esq
            generateSquare(vertices,
                x1, y0, -half,  // p1: baixo-direita
                x0, y0, -half,  // p2: baixo-esquerda
                x1, y1, -half,  // p3: cima-direita
                x0, y1, -half); // p4: cima-esquerda
            
            // Face direita (x = +half) - olhando de +X para origem
            // CCW quando visto de fora: (z0,y0), (z1,y0), (z0,y1), (z1,y1)
            generateSquare(vertices,
                half, y0, x1,   // p1: (usando x1 como z-positivo relativo)
                half, y0, x0,   // p2: (usando x0 como z-negativo relativo)
                half, y1, x1,   // p3
                half, y1, x0);  // p4
            
            // Face esquerda (x = -half) - olhando de -X para origem
            // CCW quando visto de fora: ordem inversa da direita
            generateSquare(vertices,
                -half, y0, x0,  // p1
                -half, y0, x1,  // p2
                -half, y1, x0,  // p3
                -half, y1, x1); // p4
            
            // Face superior (y = +half) - olhando de +Y para baixo
            // CCW quando visto de fora: considerando Y apontando para cima
            generateSquare(vertices,
                x0, half, y1,   // p1
                x1, half, y1,   // p2
                x0, half, y0,   // p3
                x1, half, y0);  // p4
            
            // Face inferior (y = -half) - olhando de -Y para cima
            // CCW quando visto de fora
            generateSquare(vertices,
                x0, -half, y0,  // p1
                x1, -half, y0,  // p2
                x0, -half, y1,  // p3
                x1, -half, y1); // p4
        }
    }
}

void generatePlane(float length, int divisions, list<string>& vertices) {
    float half = length / 2.0f;
    float step = length / divisions;

    for (int i = 0; i < divisions; i++) {
        for (int j = 0; j < divisions; j++) {
            float x0 = -half + i * step;
            float x1 = -half + (i + 1) * step;
            float y0 = -half + j * step;
            float y1 = -half + (j + 1) * step;

            generateSquare(vertices,
                x0, 0.0f, y0,  // p1: baixo-esquerda
                x1, 0.0f, y0,  // p2: baixo-direita
                x0, 0.0f, y1,  // p3: cima-esquerda
                x1, 0.0f, y1); // p4: cima-direita
            generateSquare(vertices,
                y0, 0.0f, x0,  // p1: baixo-esquerda
                y0, 0.0f, x1,  // p2: baixo-direita
                y1, 0.0f, x0,  // p3: cima-esquerda
                y1, 0.0f, x1); // p4: cima-direita
        }
            
    }
}

void generateSphere(float radius, int slices, int stacks, list<string>& vertices) {
}

void generateCone(float radius, float height, int slices, int stacks, list<string>& vertices) {
}

void writeOutput(const list<string>& vertices, const string& file) {
    // Construir o caminho completo: ../figures/filename.3d
    string outputPath = "../figures/" + file;
    
    ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file " << outputPath << endl;
        cerr << "Make sure the 'figures' directory exists!" << endl;
        return;
    }

    for (const auto& vertex : vertices) {
        outFile << vertex << endl;
    }

    outFile.close();
    cout << "Figure generated successfully: " << outputPath << endl;
    cout << "Total: " << vertices.size() << " vertices (" 
         << vertices.size() / 3 << " triangles)" << endl;
}


int main(int argc, char* argv[]){
    //create arglist
    list<string> arglist;
    string figure = argv[1];
    string file = argv[argc - 1];

    //split arguments
    for (int i = 2; i < argc-1; i++) {
        arglist.push_back(argv[i]);
    }

    list<string> vertices;

    if (figure == "sphere") {
        if (arglist.size() != 3) {
            cerr << "Usage: sphere <radius> <slices> <stacks> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        int slices = stoi(arglist.front());
        arglist.pop_front();
        int stacks = stoi(arglist.front());
        generateSphere(radius, slices, stacks, vertices);
    } 
    else if (figure == "box") {
        if (arglist.size() != 2) {
            cerr << "Usage: box <length> <divisions> <output_file>" << endl;
            return 1;
        }
        float length = stof(arglist.front());
        arglist.pop_front();
        int divisions = stoi(arglist.front());
        generateBox(length, divisions, vertices);
    } 
    else if (figure == "cone") {
        if (arglist.size() != 4) {
            cerr << "Usage: cone <radius> <height> <slices> <stacks> <output_file>" << endl;
            return 1;
        }
        float radius = stof(arglist.front());
        arglist.pop_front();
        float height = stof(arglist.front());
        arglist.pop_front();
        int slices = stoi(arglist.front());
        arglist.pop_front();
        int stacks = stoi(arglist.front());
        generateCone(radius, height, slices, stacks, vertices);
    } 
    else if (figure == "plane") {
        if (arglist.size() != 2) {
            cerr << "Usage: plane <length> <divisions> <output_file>" << endl;
            return 1;
        }
        float length = stof(arglist.front());
        arglist.pop_front();
        int divisions = stoi(arglist.front());
        generatePlane(length, divisions, vertices);
    } 
    else {
        cerr << "Unknown figure type: " << figure << endl;
        return 1;
    }

    writeOutput(vertices, file);

    return 0;
}
