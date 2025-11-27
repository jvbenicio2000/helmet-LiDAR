# Helmet-LiDAR

Capacete assistivo com sensor ultrassônico rotativo e feedback tátil vibratório para pessoas com deficiência visual.

## Descrição do Projeto

A ideia é fazer um capacete que consiga, através de um sensor ultrassônico que mede as distâncias e o uso de um motor de passo para movimentar este sensor 360°, acionar quatro motores que vibram e estão posicionados do lado direito, esquerdo, na frente e atrás de um usuário com deficiência visual, a fim de informá-lo se existem obstáculos na frente, atrás ou de qualquer um dos lados.

## Estrutura do Repositório

- **3D-Print:** Contém os arquivos de modelo 3D para a impressão das peças do capacete e suportes dos componentes.
- **Electronics:** Contém os esquemas elétricos, layout da placa de circuito impresso (PCI) e outras informações relacionadas à eletrônica do projeto.
- **Coding:** Contém o código-fonte para o microcontrolador (por exemplo, Arduino ou ESP32) que controla o sensor, os motores e a lógica do sistema.
- **Bill-of-Materials:** Contém a lista de todos os componentes eletrônicos e materiais necessários para a construção do projeto.

## Autores

- João Victor Benício
- Vinicius Macedo Escudeiro
- Bruno Xia
- Raul Perez Silva

### Instituição

Instituto Mauá de Tecnologia

## Matérias Relacionadas

- Instrumentação
- Microcontroladores


## Relatório do Projeto

Para uma documentação completa, incluindo a fundamentação teórica, detalhes de implementação, testes e resultados, consulte o relatório principal do projeto:

**[Relatório Completo (PDF)](./relatorio_projeto.pdf)**


## Funcionalidade Adicional: Rastreamento GPS

Além do sistema de detecção de obstáculos, o projeto inclui uma funcionalidade secundária de **rastreamento por GPS**. Utilizando uma segunda Raspberry Pi Pico, o sistema lê as coordenadas de um módulo GPS NEO-6M e as transmite via Bluetooth para um dispositivo pareado, permitindo o monitoramento remoto da localização do usuário.
