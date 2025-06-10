# ns3-consumo-energetico
 arquivos a serem modificados para rodar o modelo de consumo energético criado

 end-device-lora-mac.cc
 end-device-lora-mac.h

 lora-radio-energy-model.cc
 lora-radio-energy-model.h

 lora-tx-current-model.cc
 lora-tx-current-model.h

## end-device-lora-mac.cc / end-device-lora-mac.h
 -> Método para setar a potência de transmissão nos end devices. 
    O método chama SetTransmissionPower, deixei um //TODO pra ficar mais fácil de localizar

## lora-radio-energy-model.cc / lora-radio-energy-model.h
 -> Apenas ajuste de corrente

## lora-tx-current-model.cc / lora-tx-current-model.h
 -> Classe SX1272CurrentModel com os valores que do datasheet SX1272.
 
 -> São vários métodos, mas o mais importante aqui é o método SetTxCurrent que faz a escolha das correntes baseado na potência de transmissão
 
 -> Outro importante é o SetTxPowerToTxCurrent.

