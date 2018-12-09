
function vector = getCoeffVector(dctImage)

    vector = [];
    counter = 1;
    for i = 3:8:512
        for j = 3:8:512
            coeff = round(dctImage(i,j));
            if (coeff < 8 && coeff > -8)
                vector(counter) = coeff; 
                counter = counter + 1;
            end
        end
    end

end