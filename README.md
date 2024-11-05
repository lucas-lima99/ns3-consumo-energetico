# ns3-consumo-energetico
 arquivos a serem modificados para rodar o modelo de consumo energético criado

 end-device-lora-mac.cc
 end-device-lora-mac.h

 lora-radio-energy-model.cc
 lora-radio-energy-model.h

 lora-tx-current-model.cc
 lora-tx-current-model.h

## end-device-lora-mac.cc / end-device-lora-mac.h
 -> Criei um método para setar a potência de transmissão nos end devices. Acho que você já fez isso no seu projeto, mas decidi colocar aqui também, caso não tenha feito
    O método chama SetTransmissionPower, deixei um //TODO pra ficar mais fácil de localizar

## lora-radio-energy-model.cc / lora-radio-energy-model.h
 -> Apenas ajuste de corrente. Havia mexido aqui para o ENCOM, talvez o seu esteja igual a essa versão 


## lora-tx-current-model.cc / lora-tx-current-model.h
 -> Criei a classe SX1272CurrentModel com os valores que vi no artigo. Mais pra frente podemos atualizar pros valores do datasheet.
 -> São vários métodos, mas o mais importante aqui é o método SetTxCurrent que faz a escolha das correntes baseado na potência de transmissão
 -> Outro importante é o SetTxPowerToTxCurrent. Do jeito que estou chamando a classe no main, esse método não está sendo utilizado, mas pode ser que no futuro a gente faça mudança de potência de transmissão ao longo da simulação, então deixei ele criado pra isso

