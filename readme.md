# Babá Eletrônica – IoT

Projeto desenvolvido na disciplina de **Internet das Coisas**, aplicando conceitos de
sensoriamento, comunicação entre dispositivos, tratamento de dados e aprendizado de máquina.

---

##  Ferramentas e Tecnologias

Tecnologias utilizadas para o desenvolvimento do projeto:

- **Edge Impulse** — treinamento do modelo de machine learning
- **TensorFlow** — biblioteca de ML embarcado
- **Kaggle** — base de dados utilizada para o treinamento do modelo

---

##  Arquitetura Web

O código-fonte do projeto está organizado da seguinte forma:

- A pasta **`/SRC`** contém todo o código da aplicação
- O **front-end** foi desenvolvido em **HTML**, por se tratar de uma página estática
- O **Arduino Nano 33 BLE Sense** possui seu algoritmo implementado no arquivo  
  **`tp_arduino`**
- O **ESP32** possui seu algoritmo implementado no arquivo  
  **`tp_ESP`**
- A pasta **`/Data`** é responsável por armazenar e organizar os dados utilizados no projeto

> **Observação:**  
> O dataset utilizado não foi/será versionado neste repositório devido a limitações do Git.
> Ele está disponível publicamente na plataforma **Kaggle**.

---

## Deploy / Execução

Para executar o projeto, é necessário:

- Um **ESP32**
- Um **Arduino Nano 33 BLE Sense**

### Passos:

1. Substituir os trechos marcados com:
**&&&*&&&**
2. Carregar os códigos correspondentes em cada placa
3. Abrir o arquivo HTML por explorador ( mozila,chrome, etc )




#### Substituições necessárias:
- No arquivo **`tp_arduino`**, substituir pelo **nome** e **senha** da rede Wi-Fi
- No arquivo **HTML**, substituir pelo **caminho das imagens** localizadas na pasta **`/Data`**

---

##  Considerações Finais

Este projeto consolida os principais conceitos abordados ao longo da disciplina,
integrando **hardware**, **software**, **nuvem** e **inteligência artificial** em um sistema funcional.

