// Verwendet EXAKT die schon ausgefuehrten Lebenszyklusfunktionen. Das
// Timingprogramm und seine Messausgaben bleiben unveraendert erhalten.
#define main audit16TimingMainNotRun
#include "last_probe.cpp"
#undef main
int main(int argc,char**argv) {
    juce::ScopedJuceInitialiser_GUI gui;
    try {
        const std::string dir=argc>1?argv[1]:".";
        std::ofstream file(dir+"/resources-extended.csv");
        if(!file)throw std::runtime_error("output directory missing");
        file<<"stage,cycle,handles,threads,private_bytes,working_set_bytes\n";
        std::cout<<"Audit16LastResources build="<<__DATE__<<' '<<__TIME__<<" pid="<<GetCurrentProcessId()<<"\n";
        emitResource(file,"initial",-1);
        for(int series=0;series<3;++series) {
            std::cout<<"SERIES="<<series<<" cumulative_cycles_begin="<<(12*series)<<"\n";
            emitResource(file,"series_"+std::to_string(series)+"_before",12*series);
            cycles(file);
            messages(2000);
            emitResource(file,"series_"+std::to_string(series)+"_after2s",12*(series+1));
        }
        std::cout<<"RESOURCE_RUN_COMPLETE=36\n";return 0;
    }catch(const std::exception&e){std::cout<<"HARNESS_ERROR="<<e.what()<<"\n";return 2;}
}
