# TP Sistemas Operativos (1C - 2025)
Realizado por el Grupo Darth Kernel.
![Darth Kernel](https://drive.google.com/file/d/1zse04k85p2ukFdunid4cwAaj6mnSBS76/view?usp=sharing)

Proyecto de TP de Sistemas
Operativos de la UTN FRBA, realizado en el 1er Cuatrimestre del año 2025.

> [!WARNING]
> ### Aclaración Importante:
> El desarrollo del TP fue completado, cumpliendo con todas las funcionalidades de la consigna.
> Al momento de realizar las pruebas finales (testing), el Módulo de Kernel presentaba fallas en el planificador SJF.
> Ademas, otras funcionalidades de otros módulos no llegaron a probarse.
> Por ello, no se considera un TP aprobado.

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd kernel
make
./bin/kernel
```

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Enunciado
Enunciado completo del TP:
[Enunciado](https://docs.google.com/document/d/1HC9Zi-kpn8jI_egJGEZe77wUCbSkwSw9Ygqqs7m_-is/edit?tab=t.0)

## Testing
A continuación se adjunta link al documento de Pruebas Finales y el repositorio que contien los archivos de pseudocódigo:

- [Pruebas Fianles](https://docs.google.com/document/d/13XPliZvUBtYjaRfuVUGHWbYX8LBs8s3TDdaDa9MFr_I/edit?tab=t.0)
- [Repo Testing](https://github.com/sisoputnfrba/revenge-of-the-cth-pruebas)

## Documentación
Documentación elaborada por el grup, para acarar algunos temas en comun durante el desarrollo del mismo:
[Documentación](https://docs.google.com/document/d/1e8QvoQteYOtw42_G0_le-rf4aH5IlKBM_JWxaYAEi60/edit?tab=t.0)

## Deploy

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=io "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

> [!NOTE]
> Ante cualquier duda, pueden consultar la documentación en el repositorio de
> [so-deploy], o utilizar el comando `./deploy.sh --help`.

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy
