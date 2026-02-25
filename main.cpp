/**
 *
 * Localizar arquivos duplicados em um diretório e subdiretórios usando C++17 e Crypto++ para calcular o hash dos arquivos.
 * Este programa percorre recursivamente um diretório raiz, calcula o hash de cada arquivo e identifica arquivos duplicados com base no nome e conteúdo (hash).
 * Autor: Carlos Leonardo Goes Farias
 * Data: 20/02/2026
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <unordered_map>
#include <vector>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

std::string calcularHash(const std::filesystem::path &caminho) {
    try {
        CryptoPP::SHA256 hash;
        std::ifstream arquivo(caminho, std::ios::binary);
        if (!arquivo.is_open()) {
            std::cerr << "Não foi possível abrir o arquivo " << caminho.string() << " para leitura." << std::endl;
            return "";
        }
        constexpr size_t bufferSize = 8192;
        char buffer[bufferSize];
        while (arquivo.read(buffer, bufferSize)) {
            hash.Update(reinterpret_cast<const unsigned char *>(buffer), arquivo.gcount());
        }
        // Processa os bytes restantes
        if (arquivo.gcount() > 0) {
            hash.Update(reinterpret_cast<const unsigned char *>(buffer), arquivo.gcount());
        }
        arquivo.close();
        // Finaliza o cálculo do hash
        unsigned char digest[CryptoPP::SHA256::DIGESTSIZE];
        hash.Final(digest);
        // Converte o hash para uma string hexadecimal
        std::string hashHex;
        CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hashHex));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();
        return hashHex;
    } catch (const std::exception &e) {
        std::cerr << "Erro ao calcular o hash do arquivo " << caminho.string() << ": " << e.what() << std::endl;
        return "";
    }
}

std::unordered_map<std::string, std::vector<std::filesystem::path> > obter_arquivos_duplicados(
    const std::string &pastaRaiz) {
    std::unordered_map<std::string, std::vector<std::filesystem::path> > arquivos;
    // Percorre recursivamente o diretório raiz
    for (const auto &entrada: std::filesystem::recursive_directory_iterator(
             pastaRaiz, std::filesystem::directory_options::skip_permission_denied)) {
        if (entrada.is_regular_file()) {
            std::string nomeArquivo = entrada.path().filename().string();
            arquivos[nomeArquivo].push_back(entrada.path());
        }
    }
    return arquivos;
}

std::pair<size_t, size_t> exibir_duplicados(
    const std::unordered_map<std::string, std::vector<std::filesystem::path> > &arquivos) {
    bool encontrouDuplicados = false;
    size_t tamanhoTotalEmBytes = 0;
    int totalDuplicatas = 0;
    for (const auto &[nomeArquivo, caminhos]: arquivos) {
        if (caminhos.size() > 1) {
            // Verifica se o conteúdo dos arquivos é o mesmo usando hash
            std::unordered_map<std::string, std::vector<std::filesystem::path> > hashes;
            for (const auto &caminho: caminhos) {
                if (std::string hash = calcularHash(caminho); !hash.empty()) {
                    hashes[hash].push_back(caminho);
                }
            }
            // Imprime os arquivos duplicados
            for (const auto &caminhosHash: hashes | std::views::values) {
                if (caminhosHash.size() > 1) {
                    std::cout << "Arquivos duplicados encontrados para o nome: " << nomeArquivo << std::endl;
                    for (const auto &caminho: caminhosHash) {
                        std::cout << " - " << caminho.string() << std::endl;
                        tamanhoTotalEmBytes += std::filesystem::file_size(caminho);
                        totalDuplicatas++;
                    }
                    encontrouDuplicados = true;
                }
            }
        }
    }
    if (!encontrouDuplicados) {
        std::cout << "Nenhum arquivo duplicado encontrado." << std::endl;
    }
    return {tamanhoTotalEmBytes, totalDuplicatas};
}

int main() {
    setlocale(LC_ALL, ".UTF-8");
    std::cout << "Localizar duplicatas!" << std::endl;
    std::cout << "Informe pasta raiz: ";
    std::string pastaRaiz;
    std::getline(std::cin, pastaRaiz);
    if (!std::filesystem::exists(pastaRaiz)) {
        std::cout << "Pasta raiz não existe." << std::endl;
        return -1;
    }
    // Cria um mapa para armazenar os arquivos e seus caminhos
    std::unordered_map<std::string, std::vector<std::filesystem::path> > arquivos =
            obter_arquivos_duplicados(pastaRaiz);
    // Verifica os arquivos duplicados
    if (const auto [tamanhoBytes, total] = exibir_duplicados(arquivos); total) {
        const auto tamanhoArquivo = tamanhoBytes / total;
        // Considerando que cada arquivo duplicado tem um original e um duplicado
        std::cout << "Tamanho em bytes que pode ser liberado: " << tamanhoBytes - tamanhoArquivo << " bytes" <<
                std::endl;
    }
    return 0;
}
