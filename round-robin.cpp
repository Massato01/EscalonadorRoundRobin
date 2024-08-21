#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <locale.h>

using namespace std;

class Processo
{
public:
    string pid;
    int duracao;
    int tempo_restante;
    int tempo_chegada;
    int tempo_espera;
    vector<int> operacoes_io;

    Processo(string pid, int duracao, int tempo_chegada, vector<int> operacoes_io)
        : pid(pid), duracao(duracao), tempo_restante(duracao), tempo_chegada(tempo_chegada), tempo_espera(0), operacoes_io(operacoes_io) {}
};

vector<Processo> processos;
int quantum = 4;
int tempo_total = 0;

void imprimir_tempo(int tempo, vector<Processo> &processos_fila, vector<Processo> &processos_cpu, vector<string> &eventos)
{
    bool tem_eventos = !eventos.empty();
    bool tem_processos_na_fila = !processos_fila.empty();
    bool tem_processos_na_cpu = !processos_cpu.empty();

    cout << "=-=-= TEMPO " << tempo << " =-=-=" << endl;
    if (tem_eventos)
    {
        for (const string &evento : eventos)
        {
            cout << "#[evento] " << evento << endl;
        }
    }

    if (tem_processos_na_fila)
    {
        cout << "FILA: ";
        for (const Processo &processo : processos_fila)
        {
            cout << processo.pid << "(" << processo.tempo_restante << ") ";
        }
        cout << endl;
    }
    else
    {
        cout << "FILA: Não há processos na fila" << endl;
    }

    if (tem_processos_na_cpu)
    {
        cout << "CPU: ";
        for (const Processo &processo : processos_cpu)
        {
            cout << processo.pid << "(" << processo.tempo_restante << ") ";
        }
        cout << endl;
    }
    else
    {
        cout << "Acabaram os processos!" << endl;
    }
}

void round_robin()
{
    vector<Processo> processos_fila;
    vector<Processo> processos_cpu;
    int quantum_atual = 0;

    for (int tempo = 0; tempo <= tempo_total; ++tempo)
    {
        vector<string> eventos;

        if (tempo == 0)
        {
            processos_cpu.push_back(processos[0]);
            imprimir_tempo(tempo, processos_fila, processos_cpu, eventos);
            continue;
        }

        // Adiciona novos processos que chegaram ao tempo atual

        if (!processos_cpu.empty())
        {
            Processo &processo_atual = processos_cpu[0];

            if (processo_atual.tempo_restante > 1)
            {
                processo_atual.tempo_restante -= 1;
            }
            else
            {
                eventos.push_back("ENCERRANDO <" + processo_atual.pid + ">");
                processos_cpu.clear();
                quantum_atual = 0;
            }

            if (!processos_cpu.empty() && find(processo_atual.operacoes_io.begin(), processo_atual.operacoes_io.end(), processo_atual.duracao - processo_atual.tempo_restante) != processo_atual.operacoes_io.end())
            {
                eventos.push_back("OPERAÇÃO I/O <" + processo_atual.pid + ">");
                processos_cpu.clear();
                processos_fila.push_back(processo_atual);
                quantum_atual = 0;
            }
            quantum_atual += 1;
        }

        for (const Processo &p : processos)
        {
            if (p.tempo_chegada == tempo)
            {
                processos_fila.push_back(p);
                eventos.push_back("CHEGADA <" + p.pid + ">");
            }
        }

        if (quantum_atual > quantum && !processos_cpu.empty())
        {
            eventos.push_back("FIM QUANTUM <" + processos_cpu[0].pid + ">");
            processos_fila.push_back(processos_cpu[0]);
            processos_cpu.clear();
            quantum_atual = 0;
        }

        if (processos_cpu.empty() && !processos_fila.empty())
        {
            processos_cpu.push_back(processos_fila[0]);
            processos_fila.erase(processos_fila.begin());
        }

        if (tempo == tempo_total)
        {
            processos_cpu.clear();
        }

        imprimir_tempo(tempo, processos_fila, processos_cpu, eventos);
    }
}

void ler_arquivo(const string &caminho_arquivo)
{
    processos.clear();
    tempo_total = 0;

    ifstream arquivo(caminho_arquivo);
    string linha;
    while (getline(arquivo, linha))
    {
        istringstream ss(linha);
        string pid;
        int duracao, tempo_chegada;
        string operacoes_io_str;

        ss >> pid >> duracao >> tempo_chegada;
        vector<int> operacoes_io;

        if (ss >> operacoes_io_str)
        {
            istringstream io_ss(operacoes_io_str);
            string operacao_io;
            while (getline(io_ss, operacao_io, ','))
            {
                operacoes_io.push_back(stoi(operacao_io));
            }
        }

        Processo processo(pid, duracao, tempo_chegada, operacoes_io);
        tempo_total += duracao;
        processos.push_back(processo);
    }

    sort(processos.begin(), processos.end(), [](const Processo &a, const Processo &b)
         { return a.tempo_chegada < b.tempo_chegada; });
}

int main()
{
    setlocale(LC_ALL, "");
    cout << "=-=-= Início da simulação  =-=-=" << endl;
    cout << "Quantum: " << quantum << endl;

    ler_arquivo("input_file.txt");
    round_robin();

    cout << "=-=-= Término da simulação =-=-=" << endl;

    return 0;
}
