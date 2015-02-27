# B6-2-MPI_project
MPI project for ENSTA Paristech's B6-2 course

## Compilation

La commande suivante compile le programme

    make

La commande suivante nettoie les exécutables

    make clean

## Utilisation

Par défaut, l'algorithme bloquant est utilisé avec une taille de matrices N = 512

    mpiexec -n 4 MPI_project.x

Plusieurs arguments sont disponibles (et peuvent être combinés)

L'argument --N permet de changer la taille de matrices

    mpiexec -n 4 MPI_project.x --N 1024

L'argument --algorithm permet de changer l'algorithme utilisé (valeurs blocking ou nonblocking)

    mpiexec -n 4 MPI_project.x --algorithm nonblocking

Le flag --verbose peut être ajouté pour imprimer les matrices, résultats et erreurs (jusqu'à la taille 64)

    mpiexec -n 4 MPI_project.x --N 16 --verbose

Le flag --benchmark permet de lancer le benchmark avec les deux algorithmes et les tailles 512, 1024 et 2048

    mpiexec -n 4 MPI_project.x --benchmark
