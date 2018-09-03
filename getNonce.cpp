#include "getNonce.h"
#include "fnv.h"
#include "getTrailingZeroesCount.h"
#include "jenkins.h"
#include "pjw.h"
#include <cstring>
#include <iostream>
#include <vector>

#include <pthread.h>
#include <ctime>
#include <cmath>
#include <thread>
#include <atomic>
#include <mutex>


uint64_t uint64_max = -1;
uint64_t m = uint64_max;
//uint64_t m = 20998340 * 2;

int numberOfProcesses = 4;
std::mutex flag_mutex;
//pthread_spinlock_t flag_mutex;


void treadFunc(const std::array<uint8_t, 32>& seed, uint32_t difficulty, bool& atomic_flag,
                    uint64_t& checkedNoncesCount, uint64_t& noncefin, uint64_t nonce, uint64_t end){


    std::vector<uint8_t> data(seed.begin(), seed.end());
    data.resize(data.size() + 8);

    for (;nonce <= end; nonce++) {

        //Перед каждой проверкой nonce на валидность происходит проверка не нашел ли уже какой-нибудь из потоков
        //правильный nonce, если нашел то поток умирает не начав проверку nonce на валидность
        if(atomic_flag !=  true){

            memcpy(&data[32], &nonce, sizeof(nonce));
            uint32_t fnvHash = fnv(data.data(), data.size());
            uint32_t jenkinsHash = jenkins(data.data(), data.size());
            uint32_t pjwHash = pjw(data.data(), data.size());

            uint32_t h = fnvHash ^ jenkinsHash ^ pjwHash;
            if (getTrailingZeroesCount(h) >= difficulty) {

                //Блокировка на запись atomic_flag, checkedNoncesCount, noncefin
                flag_mutex.lock();
                //pthread_spin_lock(&flag_mutex);
                atomic_flag = true;
                checkedNoncesCount = nonce + 1;
                noncefin =  nonce;
                std::cout << "thread " << std::this_thread::get_id() << " : nonce found !!!!!!: = "<< noncefin << std::endl;
                //Снятие блокировки на запись флага
                flag_mutex.unlock();
                //pthread_spin_unlock(&flag_mutex);

                std::cout << "thread " << std::this_thread::get_id() << ": pthread_exit()" << std::endl;
                pthread_exit(NULL);
            }
        }
        else {
            std::cout << "thread " << std::this_thread::get_id() << ": pthread_exit()" << std::endl;
            pthread_exit(NULL);
        }

    }
}

uint64_t getNonce(const std::array<uint8_t, 32>& seed, uint32_t difficulty,
                  uint64_t& checkedNoncesCount)
{

  uint64_t noncefin =  0;

  //Пусть распораллеливание будет происходить при условии что difficulty >= 20 && numberOfProcesses >= 2
  if(difficulty >= 20 && numberOfProcesses >= 2) {

      //Флаг для синхронизации потоков, если поток находит нужный nonce он ставит в переменную true.
      //Используется в паре с глобальным мьютексом flag_mutex
      bool atomic_flag = false;
      //Вектор идентификаторов созданных потоков
      std::vector<std::thread*> vecThreads;
      //Делим весь диаппазон uint64_t на равные промежутки для последующей их проверки
      uint64_t step = m / numberOfProcesses;

      //Создаем numberOfProcesses потоков и разделяем между ними работу от step * k до step * (k + 1) - 1
      for (int k = 0; k < numberOfProcesses; k++) {
          if (k == numberOfProcesses - 1) {
              auto th = new std::thread(treadFunc, std::ref(seed), difficulty, std::ref(atomic_flag),
                      std::ref(checkedNoncesCount), std::ref(noncefin), step * k, uint64_max);
              vecThreads.push_back(th);
              std::cout << "Create ch-tread №" << k << '(' << th->get_id() << ')' << ": "
                        << step * k << " " << uint64_max << "\n";
          } else {
              auto th = new std::thread(treadFunc, std::ref(seed), difficulty, std::ref(atomic_flag),
                      std::ref(checkedNoncesCount),std::ref(noncefin), step * k, step * (k + 1) - 1);
              vecThreads.push_back(th);
              std::cout << "Create ch-tread №" << k << '(' << th->get_id() << ')' << ": "
                        << step * k << " " << step * (k + 1) - 1 << "\n";
          }
      }

      for (auto &th : vecThreads) {
          th->join();
          delete th;
      }
      vecThreads.clear();
  }
  else {
      //Обычный однопоточный режим работы
      std::vector<uint8_t> data(seed.begin(), seed.end());
      data.resize(data.size() + 8);
      for (uint64_t nonce = 0;; nonce++) {
          memcpy(&data[32], &nonce, sizeof(nonce));
          uint32_t fnvHash = fnv(data.data(), data.size());
          uint32_t jenkinsHash = jenkins(data.data(), data.size());
          uint32_t pjwHash = pjw(data.data(), data.size());

          uint32_t h = fnvHash ^ jenkinsHash ^ pjwHash;
          if (getTrailingZeroesCount(h) >= difficulty) {
              checkedNoncesCount = nonce + 1;
              std::cout << "nonce found !!!!!!: = "<< nonce << std::endl;
              return nonce;
          }
      }
  }

  return noncefin;
}
